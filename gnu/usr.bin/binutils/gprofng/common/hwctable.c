/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include "hwcdrv.h"
#include "hwcfuncs.h"

/* TprintfT(<level>,...) definitions.  Adjust per module as needed */
#define DBG_LT0 0 // for high-level configuration, unexpected errors/warnings
#define DBG_LT1 1 // for configuration details, warnings
#define DBG_LT2 2
#define DBG_LT3 3

/*---------------------------------------------------------------------------*/
/* compile options */

#define DISALLOW_USI_USII_6357446
/* Solaris 9/libcpc1 allows cpc_bind() to work on US-IIe processors, even
   though this processor cannot generate profiling interrupts. */

#define DISALLOW_PENTIUM_PRO_MMX_7007575
/* Solaris/libcpc2 defaults to "Pentium Pro with MMX, Pentium II"
   when it doesn't recognize an Intel processor.  As a result,
   when collect attempts to start Pentium Pro counters on a
   new machine (e.g. Westmere as of 1/2011), the OS may hang.  */

/* Register 0 counter doesn't work on Niagara T1 version (?) */
#define WORKAROUND_6231196_NIAGARA1_NO_CTR_0

/*---------------------------------------------------------------------------*/
/* consts, macros */

/* 10^N rates */
#define PRELOADS_9      1001000001
#define PRELOADS_85      320100001
#define PRELOADS_8       100100001
#define PRELOADS_75       32010001
#define PRELOADS_7        10010001
#define PRELOADS_65        3201001
#define PRELOADS_6         1001001
#define PRELOADS_55         320101
#define PRELOADS_5          100101
#define PRELOADS_45          32001
#define PRELOADS_4           10001
#define PRELOADS_35           3201
#define PRELOADS_3            1001
#define PRELOADS_25            301

#define ABST_TBD        ABST_NONE /* to be determined */

/*---------------------------------------------------------------------------*/
/* prototypes */
static void hwc_cb (uint_t cpc_regno, const char *name);
static void attrs_cb (const char *attr);
static int attr_is_valid (int forKernel, const char *attr);

/*---------------------------------------------------------------------------*/
/* HWC definition tables */

/*
  comments on hwcentry tables
  ---------------------------
  name:          this field should not contain '~'.
  int_name:      actual name of register, may contain ~ attribute specifications.
  regnum:        assigned register.
  metric:        if non-NULL, is a 'standard' counter that will show up in help.
  timecvt:       >0: can convert to time, 'timecvt' CPU cycles per event
		 =0: counts events
		 <0: can convert to time, count reference-clock cycles at '-timecvt' MHz
  memop:         see description for ABST_type enum
 */

// PRELOAD(): generates an interval based on the cycles/event and CPU GHZ.
// Note: the macro tweaks the interval so that it ends in decimal 001.
#define CYC_PER_SAMPLE (1000ULL*1000*1000/100) // cycles per signal at 1ghz, 100 samples/second
#define PRELOAD(min_cycles_per_event,ghz) (((ghz)*CYC_PER_SAMPLE/(min_cycles_per_event))/100*100+1)

// PRELOAD_DEF: initial value for uncalibrated events.
// This value should be based on a rate that will work for the slowest changing
// HWCs, HWCs where there are many CPU cycles between events.
//
// The interval needs to target the slowest HWCs so that
// automatic adjustment of HWC overflow intervals can adapt.
#define PRELOAD_DEF PRELOAD(1000,3)  // default interval targets 1000 cycles/event at 3ghz
// For er_kernel, which HWC intervals cannot be adjusted automatically for ON/HI/LO,
// The interval should target some safe interval for fast events
#define PRELOAD_DEF_ERKERNEL PRELOAD(4,4)  // default interval targets 4 cycles/event at 4ghz

static const Hwcentry empty_ctr = {NULL, NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, 0};


// --- use cycles counter to expose "system_time" on Linux ---
#define SYSTIME_REGNOS REGNO_ANY // Linux: make sys_time/usr_time available for data collection
// Note: For x86, Linux and Solaris use different ref-clock names
#define USE_INTEL_REF_CYCLES(MHZ) \
    {"usr_time","unhalted-reference-cycles",                SYSTIME_REGNOS, STXT("User CPU"),   PRELOAD(900,MHZ),  -(MHZ), ABST_NONE}, \
    {"usr_time","cpu_clk_unhalted.ref_p",                   SYSTIME_REGNOS, STXT("User CPU"),   PRELOAD(900,MHZ),  -(MHZ), ABST_NONE}, \
    {"sys_time","unhalted-reference-cycles~system=1~user=0", SYSTIME_REGNOS, STXT("System CPU"), PRELOAD(900,MHZ), -(MHZ), ABST_NONE}, \
    {"sys_time","cpu_clk_unhalted.ref_p~system=1~user=0",   SYSTIME_REGNOS, STXT("System CPU"), PRELOAD( 900,MHZ), -(MHZ), ABST_NONE}, \
    {"cycles0",	"unhalted-reference-cycles",                0,  NULL,   PRELOAD( 900,MHZ),  -(MHZ), ABST_NONE}, /*hidden*/ \
    {"cycles0",	"cpu_clk_unhalted.ref_p",                   0,  NULL,   PRELOAD( 900,MHZ),  -(MHZ), ABST_NONE}, /*hidden*/ \
    {"cycles1",	"unhalted-reference-cycles",                1,  NULL,   PRELOAD( 910,MHZ),  -(MHZ), ABST_NONE}, /*hidden*/ \
    {"cycles1",	"cpu_clk_unhalted.ref_p",                   1,  NULL,   PRELOAD( 910,MHZ),  -(MHZ), ABST_NONE}, /*hidden*/ \
    /* end of list */

#define SPARC_CYCLES \
    {"usr_time","Cycles_user",                  SYSTIME_REGNOS, STXT("User CPU"),   PRELOADS_75,1, ABST_NONE}, \
    {"sys_time","Cycles_user~system=1~user=0",  SYSTIME_REGNOS, STXT("System CPU"), PRELOADS_75,1, ABST_NONE}, \
    /* end of list */


/* --- PERF_EVENTS "software" definitions --- */
#define PERF_EVENTS_SW_EVENT_ALIASES \
// none supported for now
#if 0
    {"usr",		"PERF_COUNT_SW_TASK_CLOCK",			REGNO_ANY, STXT("User CPU"),			PRELOADS_7, -(1000), ABST_NONE}, \
    {"sys",		"PERF_COUNT_SW_TASK_CLOCK~system=1~user=0",	REGNO_ANY, STXT("System CPU"),			PRELOADS_7, -(1000), ABST_NONE}, \
    /* end of list */
#endif

#define PERF_EVENTS_SW_EVENT_DEFS \
// none supported for now
#if 0
    {"PERF_COUNT_SW_TASK_CLOCK",					NULL, REGNO_ANY, NULL, PRELOADS_7, -(1000),ABST_NONE}, \
    /* end of list */
#endif

/*
 * The PAPI descriptive strings used to be wrapped with STXT(),
 * a macro defined in perfan/include/i18n.h.  For the time being,
 * we want to demote the PAPI counters by omitting the
 * descriptions.  So we use a new macro PAPITXT() for this purpose.
 */
#define PAPITXT(x)  NULL

/* Solaris "Generic" Counters */
static Hwcentry papi_generic_list[] = {
  {"PAPI_l1_dcm", NULL, REGNO_ANY, PAPITXT ("L1 D-cache misses"), PRELOADS_65, 0, ABST_NONE},
  {"PAPI_l1_icm", NULL, REGNO_ANY, PAPITXT ("L1 I-cache misses"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_l2_dcm", NULL, REGNO_ANY, PAPITXT ("L2 D-cache misses"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_l2_icm", NULL, REGNO_ANY, PAPITXT ("L2 I-cache misses"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_l3_dcm", NULL, REGNO_ANY, PAPITXT ("L3 D-cache misses"), PRELOADS_5, 0, ABST_NONE},
  {"PAPI_l3_icm", NULL, REGNO_ANY, PAPITXT ("L3 I-cache misses"), PRELOADS_5, 0, ABST_NONE},
  {"PAPI_l1_tcm", NULL, REGNO_ANY, PAPITXT ("L1 misses"), PRELOADS_65, 0, ABST_NONE},
  {"PAPI_l2_tcm", NULL, REGNO_ANY, PAPITXT ("L2 misses"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_l3_tcm", NULL, REGNO_ANY, PAPITXT ("L3 misses"), PRELOADS_5, 0, ABST_NONE},
  {"PAPI_ca_snp", NULL, REGNO_ANY, PAPITXT ("Requests for a snoop"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_ca_shr", NULL, REGNO_ANY, PAPITXT ("Requests for exclusive access to shared cache line"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_ca_cln", NULL, REGNO_ANY, PAPITXT ("Requests for exclusive access to clean cache line"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_ca_inv", NULL, REGNO_ANY, PAPITXT ("Requests for cache line invalidation"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_ca_itv", NULL, REGNO_ANY, PAPITXT ("Requests for cache line intervention"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_l3_ldm", NULL, REGNO_ANY, PAPITXT ("L3 load misses"), PRELOADS_5, 0, ABST_NONE},
  {"PAPI_l3_stm", NULL, REGNO_ANY, PAPITXT ("L3 store misses"), PRELOADS_5, 0, ABST_NONE},
  {"PAPI_bru_idl", NULL, REGNO_ANY, PAPITXT ("Cycles branch units are idle"), PRELOADS_7, 1, ABST_NONE},
  {"PAPI_fxu_idl", NULL, REGNO_ANY, PAPITXT ("Cycles integer units are idle"), PRELOADS_7, 1, ABST_NONE},
  {"PAPI_fpu_idl", NULL, REGNO_ANY, PAPITXT ("Cycles FP units are idle"), PRELOADS_7, 1, ABST_NONE},
  {"PAPI_lsu_idl", NULL, REGNO_ANY, PAPITXT ("Cycles load/store units are idle"), PRELOADS_7, 1, ABST_NONE},
  {"PAPI_tlb_dm", NULL, REGNO_ANY, PAPITXT ("DTLB misses"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_tlb_im", NULL, REGNO_ANY, PAPITXT ("ITLB misses"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_tlb_tl", NULL, REGNO_ANY, PAPITXT ("Total TLB misses"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_tlb_tm", NULL, REGNO_ANY, PAPITXT ("Total TLB misses"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_l1_ldm", NULL, REGNO_ANY, PAPITXT ("L1 load misses"), PRELOADS_65, 0, ABST_NONE},
  {"PAPI_l1_stm", NULL, REGNO_ANY, PAPITXT ("L1 store misses"), PRELOADS_65, 0, ABST_NONE},
  {"PAPI_l2_ldm", NULL, REGNO_ANY, PAPITXT ("L2 load misses"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_l2_stm", NULL, REGNO_ANY, PAPITXT ("L2 store misses"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_btac_m", NULL, REGNO_ANY, PAPITXT ("Branch target address cache misses"), PRELOADS_5, 0, ABST_NONE},
  {"PAPI_prf_dm", NULL, REGNO_ANY, PAPITXT ("Data prefetch cache misses"), PRELOADS_65, 0, ABST_NONE},
  {"PAPI_l3_dch", NULL, REGNO_ANY, PAPITXT ("L3 D-cache hits"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_tlb_sd", NULL, REGNO_ANY, PAPITXT ("TLB shootdowns"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_csr_fal", NULL, REGNO_ANY, PAPITXT ("Failed store conditional instructions"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_csr_suc", NULL, REGNO_ANY, PAPITXT ("Successful store conditional instructions"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_csr_tot", NULL, REGNO_ANY, PAPITXT ("Total store conditional instructions"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_mem_scy", NULL, REGNO_ANY, PAPITXT ("Cycles Stalled Waiting for memory accesses"), PRELOADS_7, 1, ABST_NONE},
  {"PAPI_mem_rcy", NULL, REGNO_ANY, PAPITXT ("Cycles Stalled Waiting for memory reads"), PRELOADS_7, 1, ABST_NONE},
  {"PAPI_mem_wcy", NULL, REGNO_ANY, PAPITXT ("Cycles Stalled Waiting for memory writes"), PRELOADS_7, 1, ABST_NONE},
  {"PAPI_stl_icy", NULL, REGNO_ANY, PAPITXT ("Cycles with no instruction issue"), PRELOADS_7, 1, ABST_NONE},
  {"PAPI_ful_icy", NULL, REGNO_ANY, PAPITXT ("Cycles with maximum instruction issue"), PRELOADS_7, 1, ABST_NONE},
  {"PAPI_stl_ccy", NULL, REGNO_ANY, PAPITXT ("Cycles with no instructions completed"), PRELOADS_7, 1, ABST_NONE},
  {"PAPI_ful_ccy", NULL, REGNO_ANY, PAPITXT ("Cycles with maximum instructions completed"), PRELOADS_7, 1, ABST_NONE},
  {"PAPI_hw_int", NULL, REGNO_ANY, PAPITXT ("Hardware interrupts"), PRELOADS_5, 0, ABST_NONE},
  {"PAPI_br_ucn", NULL, REGNO_ANY, PAPITXT ("Unconditional branch instructions"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_br_cn", NULL, REGNO_ANY, PAPITXT ("Cond. branch instructions"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_br_tkn", NULL, REGNO_ANY, PAPITXT ("Cond. branch instructions taken"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_br_ntk", NULL, REGNO_ANY, PAPITXT ("Cond. branch instructions not taken"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_br_msp", NULL, REGNO_ANY, PAPITXT ("Cond. branch instructions mispredicted"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_br_prc", NULL, REGNO_ANY, PAPITXT ("Cond. branch instructions correctly predicted"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_fma_ins", NULL, REGNO_ANY, PAPITXT ("FMA instructions completed"), PRELOADS_65, 0, ABST_NONE},
  {"PAPI_tot_iis", NULL, REGNO_ANY, PAPITXT ("Instructions issued"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_tot_ins", NULL, REGNO_ANY, PAPITXT ("Instructions completed"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_int_ins", NULL, REGNO_ANY, PAPITXT ("Integer instructions"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_fp_ins", NULL, REGNO_ANY, PAPITXT ("Floating-point instructions"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_ld_ins", NULL, REGNO_ANY, PAPITXT ("Load instructions"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_sr_ins", NULL, REGNO_ANY, PAPITXT ("Store instructions"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_br_ins", NULL, REGNO_ANY, PAPITXT ("Branch instructions"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_vec_ins", NULL, REGNO_ANY, PAPITXT ("Vector/SIMD instructions"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_res_stl", NULL, REGNO_ANY, PAPITXT ("Cycles stalled on any resource"), PRELOADS_7, 1, ABST_NONE},
  {"PAPI_fp_stal", NULL, REGNO_ANY, PAPITXT ("Cycles the FP unit(s) are stalled"), PRELOADS_7, 1, ABST_NONE},
  {"PAPI_tot_cyc", NULL, REGNO_ANY, PAPITXT ("Total cycles"), PRELOADS_7, 1, ABST_NONE},
  {"PAPI_lst_ins", NULL, REGNO_ANY, PAPITXT ("Load/store instructions completed"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_syc_ins", NULL, REGNO_ANY, PAPITXT ("Sync instructions completed"), PRELOADS_65, 0, ABST_NONE},
  {"PAPI_l1_dch", NULL, REGNO_ANY, PAPITXT ("L1 D-cache hits"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_l2_dch", NULL, REGNO_ANY, PAPITXT ("L2 D-cache hits"), PRELOADS_65, 0, ABST_NONE},
  {"PAPI_l1_dca", NULL, REGNO_ANY, PAPITXT ("L1 D-cache accesses"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_l2_dca", NULL, REGNO_ANY, PAPITXT ("L2 D-cache accesses"), PRELOADS_65, 0, ABST_NONE},
  {"PAPI_l3_dca", NULL, REGNO_ANY, PAPITXT ("L3 D-cache accesses"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_l1_dcr", NULL, REGNO_ANY, PAPITXT ("L1 D-cache reads"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_l2_dcr", NULL, REGNO_ANY, PAPITXT ("L2 D-cache reads"), PRELOADS_65, 0, ABST_NONE},
  {"PAPI_l3_dcr", NULL, REGNO_ANY, PAPITXT ("L3 D-cache reads"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_l1_dcw", NULL, REGNO_ANY, PAPITXT ("L1 D-cache writes"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_l2_dcw", NULL, REGNO_ANY, PAPITXT ("L2 D-cache writes"), PRELOADS_65, 0, ABST_NONE},
  {"PAPI_l3_dcw", NULL, REGNO_ANY, PAPITXT ("L3 D-cache writes"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_l1_ich", NULL, REGNO_ANY, PAPITXT ("L1 I-cache hits"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_l2_ich", NULL, REGNO_ANY, PAPITXT ("L2 I-cache hits"), PRELOADS_65, 0, ABST_NONE},
  {"PAPI_l3_ich", NULL, REGNO_ANY, PAPITXT ("L3 I-cache hits"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_l1_ica", NULL, REGNO_ANY, PAPITXT ("L1 I-cache accesses"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_l2_ica", NULL, REGNO_ANY, PAPITXT ("L2 I-cache accesses"), PRELOADS_65, 0, ABST_NONE},
  {"PAPI_l3_ica", NULL, REGNO_ANY, PAPITXT ("L3 I-cache accesses"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_l1_icr", NULL, REGNO_ANY, PAPITXT ("L1 I-cache reads"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_l2_icr", NULL, REGNO_ANY, PAPITXT ("L2 I-cache reads"), PRELOADS_65, 0, ABST_NONE},
  {"PAPI_l3_icr", NULL, REGNO_ANY, PAPITXT ("L3 I-cache reads"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_l1_icw", NULL, REGNO_ANY, PAPITXT ("L1 I-cache writes"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_l2_icw", NULL, REGNO_ANY, PAPITXT ("L2 I-cache writes"), PRELOADS_65, 0, ABST_NONE},
  {"PAPI_l3_icw", NULL, REGNO_ANY, PAPITXT ("L3 I-cache writes"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_l1_tch", NULL, REGNO_ANY, PAPITXT ("L1 total hits"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_l2_tch", NULL, REGNO_ANY, PAPITXT ("L2 total hits"), PRELOADS_65, 0, ABST_NONE},
  {"PAPI_l3_tch", NULL, REGNO_ANY, PAPITXT ("L3 total hits"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_l1_tca", NULL, REGNO_ANY, PAPITXT ("L1 total accesses"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_l2_tca", NULL, REGNO_ANY, PAPITXT ("L2 total accesses"), PRELOADS_65, 0, ABST_NONE},
  {"PAPI_l3_tca", NULL, REGNO_ANY, PAPITXT ("L3 total accesses"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_l1_tcr", NULL, REGNO_ANY, PAPITXT ("L1 total reads"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_l2_tcr", NULL, REGNO_ANY, PAPITXT ("L2 total reads"), PRELOADS_65, 0, ABST_NONE},
  {"PAPI_l3_tcr", NULL, REGNO_ANY, PAPITXT ("L3 total reads"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_l1_tcw", NULL, REGNO_ANY, PAPITXT ("L1 total writes"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_l2_tcw", NULL, REGNO_ANY, PAPITXT ("L2 total writes"), PRELOADS_65, 0, ABST_NONE},
  {"PAPI_l3_tcw", NULL, REGNO_ANY, PAPITXT ("L3 total writes"), PRELOADS_6, 0, ABST_NONE},
  {"PAPI_fml_ins", NULL, REGNO_ANY, PAPITXT ("FP multiply instructions"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_fad_ins", NULL, REGNO_ANY, PAPITXT ("FP add instructions"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_fdv_ins", NULL, REGNO_ANY, PAPITXT ("FP divide instructions"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_fsq_ins", NULL, REGNO_ANY, PAPITXT ("FP square root instructions"), PRELOADS_65, 0, ABST_NONE},
  {"PAPI_fnv_ins", NULL, REGNO_ANY, PAPITXT ("FP inverse instructions"), PRELOADS_7, 0, ABST_NONE},
  {"PAPI_fp_ops", NULL, REGNO_ANY, PAPITXT ("FP operations"), PRELOADS_7, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

static Hwcentry usIlist[] = {
  {"cycles", "Cycle_cnt", REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_7, 1, ABST_NONE},
  {"insts", "Instr_cnt", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_7, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

static Hwcentry usIIIlist[] = /* III, IIIi, IIIp.  Note that some counters are processor-specific */{
  {"cycles", "Cycle_cnt", REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_7, 1, ABST_NONE},
  {"insts", "Instr_cnt", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_7, 0, ABST_NONE},
  {"icm", "IC_miss", REGNO_ANY, STXT ("I$ Misses"), PRELOADS_5, 0, ABST_NONE},
  {"dcrm", "DC_rd_miss", REGNO_ANY, STXT ("D$ Read Misses"), PRELOADS_5, 0, ABST_LOAD},
  {"dcwm", "DC_wr_miss", REGNO_ANY, STXT ("D$ Write Misses"), PRELOADS_5, 0, ABST_STORE},
  {"dcr", "DC_rd", REGNO_ANY, STXT ("D$ Read Refs"), PRELOADS_6, 0, ABST_LOAD},
  {"dcw", "DC_wr", REGNO_ANY, STXT ("D$ Write Refs"), PRELOADS_6, 0, ABST_STORE},
  {"ecref", "EC_ref", REGNO_ANY, STXT ("E$ Refs"), PRELOADS_6, 0, ABST_LDST},
  {"itlbm", "ITLB_miss", REGNO_ANY, STXT ("ITLB Misses"), PRELOADS_5, 0, ABST_NONE},
  {"dtlbm", "DTLB_miss", REGNO_ANY, STXT ("DTLB Misses"), PRELOADS_5, 0, ABST_US_DTLBM},
  {"ecm", "EC_misses", REGNO_ANY, STXT ("E$ Misses"), PRELOADS_5, 0, ABST_LDST},
  {"ecrm", "EC_rd_miss", REGNO_ANY, STXT ("E$ Read Misses"), PRELOADS_5, 0, ABST_LOAD},
  {"ecml", "EC_miss_local", REGNO_ANY, STXT ("E$ Local Misses"), PRELOADS_5, 0, ABST_LDST},
  {"ecmr", "EC_miss_remote", REGNO_ANY, STXT ("E$ Remote Misses"), PRELOADS_5, 0, ABST_LDST},
  {"ecim", "EC_ic_miss", REGNO_ANY, STXT ("E$ Instr. Misses"), PRELOADS_5, 0, ABST_NONE},
  {"icstall", "Dispatch0_IC_miss", REGNO_ANY, STXT ("I$ Stall Cycles"), PRELOADS_6, 1, ABST_NONE},
  {"dcstall", "Re_DC_miss", REGNO_ANY, STXT ("D$ and E$ Stall Cycles"), PRELOADS_6, 1, ABST_LOAD},
  {"ecstall", "Re_EC_miss", REGNO_ANY, STXT ("E$ Stall Cycles"), PRELOADS_6, 1, ABST_LOAD},
  {"sqstall", "Rstall_storeQ", REGNO_ANY, STXT ("StoreQ Stall Cycles"), PRELOADS_6, 1, ABST_STORE},
  {"rawstall", "Re_RAW_miss", REGNO_ANY, STXT ("RAW Stall Cycles"), PRELOADS_6, 1, ABST_LOAD},
  {"dcmissov", "Re_DC_missovhd", REGNO_ANY, STXT ("DC Miss Ovhd"), PRELOADS_6, 1, ABST_LOAD},
  {"fpustall", "Re_FPU_bypass", REGNO_ANY, STXT ("FPU Stall Cycles"), PRELOADS_6, 1, ABST_NONE},
  {"fpusestall", "Rstall_FP_use", REGNO_ANY, STXT ("FPU Use Stall Cycles"), PRELOADS_6, 1, ABST_NONE},
  {"iustall", "Rstall_IU_use", REGNO_ANY, STXT ("IU Stall Cycles"), PRELOADS_6, 1, ABST_NONE},
  {"fpadd", "FA_pipe_completion", REGNO_ANY, STXT ("FP Adds"), PRELOADS_6, 0, ABST_NONE},
  {"fpmul", "FM_pipe_completion", REGNO_ANY, STXT ("FP Muls"), PRELOADS_6, 0, ABST_NONE},

  /* explicit definitions of (hidden) entries for proper counters */
  /*  Only counters that can be time converted, or are load-store need to be in this table */
  {"Cycle_cnt", NULL, REGNO_ANY, NULL, PRELOADS_7, 1, ABST_NONE},
  {"EC_miss_mtag_remote", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LDST},
  {"DC_rd_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LOAD},
  {"DC_wr_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_STORE},
  {"DC_rd", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LOAD},
  {"DC_wr", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_STORE},
  {"EC_ref", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LDST},
  {"EC_snoop_inv", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC /*?*/},
  {"EC_wb", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_STORE},
  {"EC_wb_remote", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_STORE},
  {"DTLB_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_US_DTLBM},
  {"EC_misses", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LDST},
  {"EC_rd_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LOAD},
  {"PC_port0_rd", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LOAD},
  {"EC_miss_local", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LDST},
  {"EC_miss_remote", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LDST},
  {"EC_snoop_cb", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC /*?*/},
  {"WC_snoop_cb", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC /*?*/},
  {"WC_scrubbed", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_STORE},
  {"WC_wb_wo_read", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_STORE},
  {"PC_MS_misses", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LOAD},
  {"PC_soft_hit", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LOAD},
  {"PC_hard_hit", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LOAD},
  {"PC_port1_rd", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LOAD},
  {"PC_snoop_inv", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_STORE /*?*/},
  {"SW_count_0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_COUNT},
  {"SW_count_1", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_COUNT},
  {"Dispatch0_IC_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Dispatch0_mispred", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Dispatch0_br_target", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Dispatch0_2nd_br", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Dispatch_rs_mispred", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Rstall_storeQ", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_STORE},
  {"Rstall_FP_use", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Rstall_IU_use", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"EC_write_hit_RTO", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_STORE},
  {"Re_RAW_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_LOAD},
  {"Re_DC_missovhd", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_LOAD},
  {"Re_endian_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_LOAD},
  {"Re_FPU_bypass", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Re_DC_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_LOAD},
  {"Re_EC_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_LOAD},
  {"Re_PC_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_LOAD},
  {"SI_snoop", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC},
  {"SI_ciq_flow", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC},
  {"SI_owned", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC},
  {"MC_msl_busy_stall", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NOPC},
  {"MC_mdb_overflow_stall", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NOPC},
  {"MC_page_close_stall", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NOPC},
  {"MC_reads_0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC},
  {"MC_reads_1", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC},
  {"MC_reads_2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC},
  {"MC_reads_3", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC},
  {"MC_writes_0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC},
  {"MC_writes_1", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC},
  {"MC_writes_2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC},
  {"MC_writes_3", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC},
  {"MC_stalls_0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NOPC},
  {"MC_stalls_1", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NOPC},
  {"MC_stalls_2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NOPC},
  {"MC_stalls_3", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NOPC},

  /* additional (hidden) aliases, for convenience */
  {"cycles0", "Cycle_cnt", 0, NULL, PRELOADS_75, 1, ABST_NONE},
  {"cycles1", "Cycle_cnt", 1, NULL, PRELOADS_75, 1, ABST_NONE},
  {"insts0", "Instr_cnt", 0, NULL, PRELOADS_75, 0, ABST_NONE},
  {"insts1", "Instr_cnt", 1, NULL, PRELOADS_75, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

static Hwcentry usIVplist[] = {
  {"cycles", "Cycle_cnt", REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_7, 1, ABST_NONE},
  {"insts", "Instr_cnt", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_7, 0, ABST_NONE},
  {"icm", "IC_fill", REGNO_ANY, STXT ("I$ Misses"), PRELOADS_5, 0, ABST_NONE},
  {"dcrm", "DC_rd_miss", REGNO_ANY, STXT ("D$ Read Misses"), PRELOADS_5, 0, ABST_LOAD},
  {"dcwm", "DC_wr_miss", REGNO_ANY, STXT ("D$ Write Misses"), PRELOADS_5, 0, ABST_STORE},
  {"dcr", "DC_rd", REGNO_ANY, STXT ("D$ Read Refs"), PRELOADS_6, 0, ABST_LOAD},
  {"dcw", "DC_wr", REGNO_ANY, STXT ("D$ Write Refs"), PRELOADS_6, 0, ABST_STORE},
  {"itlbm", "ITLB_miss", REGNO_ANY, STXT ("ITLB Misses"), PRELOADS_5, 0, ABST_NONE},
  {"dtlbm", "DTLB_miss", REGNO_ANY, STXT ("DTLB Misses"), PRELOADS_5, 0, ABST_US_DTLBM},
  {"l2ref", "L2_ref", REGNO_ANY, STXT ("L2$ Refs"), PRELOADS_5, 0, ABST_LDST},
  {"l2m", "L2_miss", REGNO_ANY, STXT ("L2$ Misses"), PRELOADS_5, 0, ABST_LDST},
  {"l2rm", "L2_rd_miss", REGNO_ANY, STXT ("L2$ Read Misses"), PRELOADS_5, 0, ABST_LOAD},
  {"l2im", "L2_IC_miss", REGNO_ANY, STXT ("L2$ Instr. Misses"), PRELOADS_5, 0, ABST_NONE},
  {"ecm", "L3_miss", REGNO_ANY, STXT ("E$ Misses"), PRELOADS_5, 0, ABST_LDST},
  {"ecrm", "L3_rd_miss", REGNO_ANY, STXT ("E$ Read Misses"), PRELOADS_5, 0, ABST_LOAD},
  {"ecml", "SSM_L3_miss_local", REGNO_ANY, STXT ("E$ Local Misses"), PRELOADS_5, 0, ABST_LDST},
  {"ecmr", "SSM_L3_miss_remote", REGNO_ANY, STXT ("E$ Remote Misses"), PRELOADS_5, 0, ABST_LDST},
  {"ecim", "L3_IC_miss", REGNO_ANY, STXT ("E$ Instr. Misses"), PRELOADS_5, 0, ABST_NONE},
  {"icstall", "Dispatch0_IC_miss", REGNO_ANY, STXT ("I$ Stall Cycles"), PRELOADS_6, 1, ABST_NONE},
  {"dcstall", "Re_DC_miss", REGNO_ANY, STXT ("D$ and E$ Stall Cycles"), PRELOADS_6, 1, ABST_LOAD},
  {"ecstall", "Re_L3_miss", REGNO_ANY, STXT ("E$ Stall Cycles"), PRELOADS_6, 1, ABST_LOAD},
  {"sqstall", "Rstall_storeQ", REGNO_ANY, STXT ("StoreQ Stall Cycles"), PRELOADS_6, 1, ABST_STORE},
  {"rawstall", "Re_RAW_miss", REGNO_ANY, STXT ("RAW Stall Cycles"), PRELOADS_6, 1, ABST_LOAD},
  {"dcmissov", "Re_DC_missovhd", REGNO_ANY, STXT ("DC Miss Ovhd"), PRELOADS_6, 1, ABST_LOAD},
  {"fpustall", "Re_FPU_bypass", REGNO_ANY, STXT ("FPU Stall Cycles"), PRELOADS_6, 1, ABST_NONE},
  {"fpusestall", "Rstall_FP_use", REGNO_ANY, STXT ("FPU Use Stall Cycles"), PRELOADS_6, 1, ABST_NONE},
  {"iustall", "Rstall_IU_use", REGNO_ANY, STXT ("IU Stall Cycles"), PRELOADS_6, 1, ABST_NONE},
  {"fpadd", "FA_pipe_completion", REGNO_ANY, STXT ("FP Adds"), PRELOADS_6, 0, ABST_NONE},
  {"fpmul", "FM_pipe_completion", REGNO_ANY, STXT ("FP Muls"), PRELOADS_6, 0, ABST_NONE},

  /* explicit definitions of (hidden) entries for proper counters */
  /*  Only counters that can be time converted, or are load-store need to be in this table */
  {"Cycle_cnt", NULL, REGNO_ANY, NULL, PRELOADS_7, 1, ABST_NONE},
  {"DC_rd", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LOAD},
  {"DC_rd_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LOAD},
  {"DC_wr", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_STORE},
  {"DC_wr_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_STORE},
  {"DTLB_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_US_DTLBM},
  {"Dispatch0_2nd_br", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Dispatch0_IC_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Dispatch0_other", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2L3_snoop_cb_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC /*?*/},
  {"L2L3_snoop_inv_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC /*?*/},
  {"L2_hit_I_state_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LDST /*?*/},
  {"L2_hit_other_half", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LDST},
  {"L2_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LDST},
  {"L2_rd_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LOAD},
  {"L2_ref", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LDST},
  {"L2_snoop_cb_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC /*?*/},
  {"L2_snoop_inv_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC /*?*/},
  {"L2_wb", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_STORE},
  {"L2_wb_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_STORE},
  {"L2_write_hit_RTO", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_STORE},
  {"L2_write_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_STORE},
  {"L3_hit_I_state_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LDST},
  {"L3_hit_other_half", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LDST},
  {"L3_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LDST},
  {"L3_rd_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LOAD},
  {"L3_wb", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_STORE},
  {"L3_wb_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_STORE},
  {"L3_write_hit_RTO", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_STORE},
  {"L3_write_miss_RTO", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_STORE},
  {"MC_reads_0_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC},
  {"MC_reads_1_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC},
  {"MC_reads_2_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC},
  {"MC_reads_3_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC},
  {"MC_stalls_0_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NOPC},
  {"MC_stalls_1_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NOPC},
  {"MC_stalls_2_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NOPC},
  {"MC_stalls_3_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NOPC},
  {"MC_writes_0_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC},
  {"MC_writes_1_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC},
  {"MC_writes_2_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC},
  {"MC_writes_3_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC},
  /*? {"PC_MS_misses",	NULL, REGNO_ANY, NULL, PRELOAD_DEF,     0, ABST_LOAD}, */
  {"PC_hard_hit", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LOAD},
  {"PC_inv", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_STORE /*?*/},
  {"PC_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LOAD},
  {"PC_rd", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LOAD},
  {"PC_soft_hit", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LOAD},
  {"Re_DC_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_LOAD},
  {"Re_DC_missovhd", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_LOAD},
  {"Re_FPU_bypass", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Re_L2_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_LOAD},
  {"Re_L3_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_LOAD},
  {"Re_PFQ_full", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Re_RAW_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_LOAD},
  {"Rstall_FP_use", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Rstall_IU_use", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Rstall_storeQ", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_STORE},
  {"SI_RTO_src_data", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC},
  {"SI_RTS_src_data", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC},
  {"SI_ciq_flow_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NOPC},
  {"SI_owned_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC},
  {"SI_snoop_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NOPC},
  {"ecml", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LDST},
  {"ecmr", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LDST},
  {"SSM_L3_miss_local", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LDST /*?*/},
  {"SSM_L3_miss_mtag_remote", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LDST /*?*/},
  {"SSM_L3_miss_remote", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_LDST /*?*/},
  {"SSM_L3_wb_remote", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_STORE /*?*/},
  {"SSM_new_transaction_sh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_TBD /*?*/},

  /* additional (hidden) aliases, for convenience */
  {"cycles0", "Cycle_cnt", 0, NULL, PRELOADS_75, 1, ABST_NONE},
  {"cycles1", "Cycle_cnt", 1, NULL, PRELOADS_75, 1, ABST_NONE},
  {"insts0", "Instr_cnt", 0, NULL, PRELOADS_75, 0, ABST_NONE},
  {"insts1", "Instr_cnt", 1, NULL, PRELOADS_75, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

static Hwcentry niagara1[] =
	/* CPC_ULTRA_T1              , "UltraSPARC T1" */{
  {"insts", "Instr_cnt", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_7, 0, ABST_NONE},
#ifndef WORKAROUND_6231196_NIAGARA1_NO_CTR_0 /* since register 0 counter don't work XXX */
  {"icm", "IC_miss", REGNO_ANY, STXT ("I$ Misses"), PRELOADS_5, 0, ABST_NONE},
  {"itlbm", "ITLB_miss", REGNO_ANY, STXT ("ITLB Misses"), PRELOADS_5, 0, ABST_NONE},
  {"ecim", "L2_imiss", REGNO_ANY, STXT ("E$ Instr. Misses"), PRELOADS_4, 0, ABST_NONE},
  {"dcm", "DC_miss", REGNO_ANY, STXT ("D$ Misses"), PRELOADS_5, 0, ABST_EXACT},
  {"dtlbm", "DTLB_miss", REGNO_ANY, STXT ("DTLB Misses"), PRELOADS_5, 0, ABST_EXACT},
  {"ecdm", "L2_dmiss_ld", REGNO_ANY, STXT ("E$ Data Misses"), PRELOADS_4, 0, ABST_EXACT},
  {"flops", "FP_instr_cnt", REGNO_ANY, STXT ("Floating-point Ops"), PRELOADS_6, 0, ABST_NONE},

  /* explicit definitions of (hidden) entries for proper counters */
  /*  Only counters that can be time converted, or are load-store need to be in this table */
  {"SB_full", NULL, REGNO_ANY, NULL, PRELOADS_6, 1, ABST_NONE},
  {"DC_miss", NULL, REGNO_ANY, NULL, PRELOADS_6, 0, ABST_EXACT},
  {"DTLB_miss", NULL, REGNO_ANY, NULL, PRELOADS_6, 0, ABST_EXACT},
  {"L2_dmiss_ld", NULL, REGNO_ANY, NULL, PRELOADS_6, 0, ABST_EXACT},
#endif

  /* additional (hidden) aliases, for convenience */
  {"insts1", "Instr_cnt", 1, NULL, PRELOADS_75, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

static Hwcentry niagara2[] = {
  /* CPC_ULTRA_T2              , "UltraSPARC T2" */
  /* CPC_ULTRA_T2              , "UltraSPARC T2+" */
  {"insts", "Instr_cnt", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_7, 0, ABST_NONE},
  {"loads", "Instr_ld", REGNO_ANY, STXT ("Load Instructions"), PRELOADS_7, 0, ABST_EXACT},
  {"stores", "Instr_st", REGNO_ANY, STXT ("Store Instructions"), PRELOADS_6, 0, ABST_EXACT},
  {"dcm", "DC_miss", REGNO_ANY, STXT ("L1 D-cache Misses"), PRELOADS_6, 0, ABST_EXACT},
  {"dtlbm", "DTLB_miss", REGNO_ANY, STXT ("DTLB Misses"), PRELOADS_6, 0, ABST_NONE},
  {"l2drm", "L2_dmiss_ld", REGNO_ANY, STXT ("L2 D-cache Read Misses (See Bug 15664448)"), PRELOADS_5, 0, ABST_EXACT},
  {"icm", "IC_miss", REGNO_ANY, STXT ("L1 I-cache Misses"), PRELOADS_5, 0, ABST_NONE},
  {"itlbm", "ITLB_miss", REGNO_ANY, STXT ("ITLB Misses"), PRELOADS_5, 0, ABST_NONE},
  {"l2im", "L2_imiss", REGNO_ANY, STXT ("L2 I-cache Misses"), PRELOADS_4, 0, ABST_NONE},

  /* explicit definitions of (hidden) entries for proper counters */
  /* Only counters that can be time converted, or are load-store need to be in this table */
  {"Instr_ld", NULL, REGNO_ANY, NULL, PRELOADS_7, 0, ABST_EXACT},
  {"Instr_st", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT},
  {"Atomics", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT},
  {"DC_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT},
  {"L2_dmiss_ld", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT},
  {"DTLB_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE},
  {"DES_3DES_busy_cycle", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"AES_busy_cycle", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Kasumi_busy_cycle", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"MD5_SHA-1_SHA-256_busy_cycle", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"MA_busy_cycle", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  /* additional (hidden) aliases, for convenience */
  {"insts1", "Instr_cnt", 1, NULL, PRELOADS_75, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

static Hwcentry sparc_t4[] = {
  // Identical to sparc_t5_m6 except for: l3m_spec
  // when updating this table, also update sparc_t5_m6[]
  // obsolete aliases marked with REGNO_INVALID (allows reading of older experiments)
  {"l2l3dh", "DC_miss_L2_L3_hit_nospec", REGNO_INVALID, STXT ("L2 or L3 D-cache Hits"), PRELOADS_6, 0, ABST_EXACT}, // undercounts due to thread-hog issue
  {"l3m", "DC_miss_remote_L3_hit_nospec~emask=0x6", REGNO_INVALID, STXT ("L3 D-cache Misses"), PRELOADS_5, 0, ABST_EXACT}, // undercounts due to thread-hog issue
  {"lmh", "DC_miss_local_hit_nospec", REGNO_INVALID, STXT ("Local Mem. Hits"), PRELOADS_5, 0, ABST_EXACT}, // undercounts due to thread-hog issue
  {"rmh", "DC_miss_remote_L3_hit_nospec", REGNO_INVALID, STXT ("Remote Mem. Hits"), PRELOADS_5, 0, ABST_EXACT}, // undercounts due to thread-hog issue
  {"pqs", "PQ_tag_wait", REGNO_INVALID, STXT ("Pick Queue Stalls"), PRELOADS_7, 1, ABST_NONE}, // old alias name
  {"raw_stb", "RAW_hit_st_buf", REGNO_INVALID, STXT ("RAW Hazard in Store Buffer"), PRELOADS_55, 0, ABST_NONE}, // 11@full hit, 60@partial hit (in future, combine w/st_q)
  {"raw_stq", "RAW_hit_st_q", REGNO_INVALID, STXT ("RAW Hazard in Store Queue"), PRELOADS_55, 0, ABST_NONE}, // 11@full hit, 60@partial hit (in future, combine w/st_buf)
  {"sel_stalls", "Sel_0_ready", REGNO_INVALID, STXT ("Stalls Another Thread Selected"), PRELOADS_7, 1, ABST_NONE},
  {"icm", "IC_miss", REGNO_INVALID, STXT ("L1 I-Cache Misses"), PRELOADS_55, 0, ABST_NONE}, // 20@ l2/l3 hit (guess)
  {"icm_stalls", "IC_miss", REGNO_INVALID, STXT ("L1 I-Cache Miss Est Stalls"), PRELOADS_55, 25, ABST_NONE}, // 25@ l2-20/l3-50

  // current aliases
  SPARC_CYCLES
  {"cycles", "Cycles_user", REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_75, 1, ABST_NONE},
  {"insts", "Instr_all", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_75, 0, ABST_NONE},
  {"c_stalls", "Commit_0", REGNO_ANY, STXT ("Stall Cycles"), PRELOADS_7, 1, ABST_NONE},
  {"loads", "Instr_ld", REGNO_ANY, STXT ("Load Instructions"), PRELOADS_7, 0, ABST_EXACT},
  {"stores", "Instr_st", REGNO_ANY, STXT ("Store Instructions"), PRELOADS_7, 0, ABST_EXACT},
  {"dcm", "DC_miss_nospec", REGNO_ANY, STXT ("L1 D-cache Misses"), PRELOADS_65, 0, ABST_EXACT},
  {"l3m_spec", "DC_miss_local_hit~emask=0x6", REGNO_ANY, STXT ("L3 D-cache Speculative Misses"), PRELOADS_5, 0, ABST_NONE, STXT ("Loads that speculatively missed local L3")}, // T4 encoding (430 lm, 690 rm) ~5 misses overlap on t5/pico_ile
  //  {"l3m_spec",	"DC_miss_local_hit~emask=0x30",		REGNO_ANY, STXT("L3 D-cache Speculative Misses"),PRELOADS_5,0, ABST_NONE, STXT("Loads that speculatively missed local L3")}, // T5/M6 encoding (430 lm, 690 rm) ~5 misses overlap on t5/pico_ile
  {"lmh_spec", "DC_miss_local_hit", REGNO_ANY, STXT ("Local Mem Speculative Hits"), PRELOADS_5, 0, ABST_NONE},
  {"rmh_spec", "DC_miss_remote_L3_hit", REGNO_ANY, STXT ("Remote Mem Speculative Hits"), PRELOADS_5, 0, ABST_NONE},
  //
  {"dtlbm", "DTLB_miss_asynch", REGNO_ANY, STXT ("DTLB Misses"), PRELOADS_55, 0, ABST_NONE}, // 10@l1 hit, 24@l2 hit, 60@l3 hit, 500@l3 miss, 5000@trap  0.001 events/cycle
  {"dtlb_hwtw_stalls", "DTLB_HWTW_all", REGNO_ANY, STXT ("DTLB HWTW Est Stalls"), PRELOADS_55, 25, ABST_NONE, STXT ("Estimated time stalled on a DTLB miss requiring a HW tablewalk")}, // l2-20, l3-50
  {"dtlb_trap_stalls", "DTLB_fill_trap", REGNO_ANY, STXT ("DTLB Trap Est Stalls"), PRELOADS_35, 5000, ABST_NONE, STXT ("Estimated time stalled on a DTLB miss with HW tablewalk unsuccessful")}, // 5000@trap
  {"rawhaz", "RAW_hit_st_q~emask=0xf", REGNO_ANY, STXT ("Read-after-write Hazards"), PRELOADS_55, 0, ABST_NONE, STXT ("Loads delayed by a previous store (read-after-write hazards)")},
  {"br_msp_stalls", "Br_mispred", REGNO_ANY, STXT ("Branch Mispredict Stalls"), PRELOADS_6, 24, ABST_NONE, STXT ("Estimated time stalled on Branch mispredictions")}, // 24@miss, %5 of branches is bad
  {"br_msp", "Br_mispred", REGNO_ANY, STXT ("Branch Mispredict"), PRELOADS_6, 0, ABST_NONE}, // 24@miss, %5 of branches is bad
  {"br_tkn", "Br_taken", REGNO_ANY, STXT ("Branch Taken"), PRELOADS_7, 0, ABST_NONE}, // 2 cycles minimum
  {"br_ins", "Branches", REGNO_ANY, STXT ("Branch Instructions"), PRELOADS_7, 0, ABST_NONE}, // 24@miss, %5 of branches is bad
  {"fgu", "Instr_FGU_crypto", REGNO_ANY, STXT ("FP/VIS/Crypto Instructions"), PRELOADS_7, 0, ABST_NONE}, // 1 cycle/event

  /* explicit definitions of (hidden) entries for proper counters */
  /* Counters that can be time converted, support memspace, or have a short_desc need to be in this table */

  {"Sel_pipe_drain_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles a hardware thread stalls at Select waiting with correct instructions when pipeline has to drain after branch misprediction")},
  {"Sel_0_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles a hardware thread stalls at Select waiting for various conditions to be resolved")},
  {"Sel_0_ready", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles a hardware thread was ready to have its instructions selected but another hardware thread was selected instead")},
  {"Sel_1", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles that only 1 instruction or uop was selected")},
  {"Sel_2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles that 2 instructions or uops were selected")},

  {"Pick_0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Pick_1", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Pick_2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Pick_3", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Pick_any", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {"Branches", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("Control transfer instructions completed, excluding trap-related transfers")},
  {"Instr_FGU_crypto", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("FP and VIS instructions completed by the Floating Point and Graphics Unit")},
  {"Instr_ld", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT, STXT ("Load instructions completed")},
  {"Instr_st", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT, STXT ("Store instructions completed")},
  {"SPR_ring_ops", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT, STXT ("Specialized instructions that require internal use of SPR ring completed")},
  {"Instr_other", NULL, REGNO_ANY, NULL, PRELOAD (2, 4), 0, ABST_NONE, STXT ("Basic arithmetic and logical instructions completed")},
  {"Instr_all", NULL, REGNO_ANY, NULL, PRELOAD (1, 4), 0, ABST_NONE, STXT ("Total instructions completed")},

  {"Br_taken", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("Branch instructions taken and completed")},
  {"Sw_count_intr", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("SW Count instructions completed")},
  {"Atomics", NULL, REGNO_ANY, NULL, PRELOAD (20, 4), 0, ABST_EXACT, STXT ("Atomic instructions, including CASA/XA, completed")},
  {"SW_prefetch", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT, STXT ("PREFETCH and PREFETCHA instructions completed")},
  {"Block_ld_st", NULL, REGNO_ANY, NULL, PRELOAD (20, 4), 0, ABST_EXACT, STXT ("Block load/store instructions completed")},

  {"BTC_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("Branches delayed a few extra cycles because branch target not found in Branch Target Cache")},

  {"ITLB_fill_8KB", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk successfully loaded translation for 8K page")},
  {"ITLB_fill_64KB", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk successfully loaded translation for 64K page")},
  {"ITLB_fill_4MB", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk successfully loaded translation for 4M page")},
  {"ITLB_fill_256MB", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk successfully loaded translation for 256M page")},
  {"ITLB_fill_2GB", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk successfully loaded translation for 2G or 16G page")},
  {"ITLB_fill_trap", NULL, REGNO_ANY, NULL, PRELOAD (1000, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk unsuccessful")},
  {"ITLB_miss_asynch", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk search done")},

  {"Fetch_0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Fetch_0_all", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {"Instr_buffer_full", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {"PQ_tag_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"ROB_tag_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"LB_tag_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"ROB_LB_tag_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"SB_tag_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"ROB_SB_tag_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"LB_SB_tag_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"ROB_LB_SB_tag_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"DTLB_miss_tag_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {"ITLB_HWTW_L2_hit", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk hit local L2D")},
  {"ITLB_HWTW_L3_hit", NULL, REGNO_ANY, NULL, PRELOAD (80, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk hit local L3 or neighbor L2D")},
  {"ITLB_HWTW_L3_miss", NULL, REGNO_ANY, NULL, PRELOAD (800, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk missed all local caches")},
  {"DTLB_HWTW_L2_hit", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk hit local L2D")},
  {"DTLB_HWTW_L3_hit", NULL, REGNO_ANY, NULL, PRELOAD (80, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk hit local L3 or neighbor L2D")},
  {"DTLB_HWTW_L3_miss", NULL, REGNO_ANY, NULL, PRELOAD (800, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk missed all local caches")},
  {"DTLB_HWTW_all", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss requiring HW tablewalk")},

  {"DC_miss_L2_L3_hit_nospec", NULL, REGNO_ANY, NULL, PRELOAD (25, 4), 0, ABST_EXACT},
  {"DC_miss_local_hit_nospec", NULL, REGNO_ANY, NULL, PRELOAD (500, 4), 0, ABST_EXACT},
  {"DC_miss_remote_L3_hit_nospec", NULL, REGNO_ANY, NULL, PRELOAD (800, 4), 0, ABST_EXACT},
  {"DC_miss_nospec", NULL, REGNO_ANY, NULL, PRELOAD (25, 4), 0, ABST_EXACT, STXT ("Loads that missed local L1D")},

  {"DTLB_fill_8KB", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk successfully loaded translation for 8K page")},
  {"DTLB_fill_64KB", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk successfully loaded translation for 64K page")},
  {"DTLB_fill_4MB", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk successfully loaded translation for 4M page")},
  {"DTLB_fill_256MB", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk successfully loaded translation for 256M page")},
  {"DTLB_fill_2GB", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk successfully loaded translation for 2G or 16G page")},
  {"DTLB_fill_trap", NULL, REGNO_ANY, NULL, PRELOAD (800, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk unsuccessful")},
  {"DTLB_miss_asynch", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk search done")},
  {"RAW_hit_st_buf", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("Loads delayed by a previous store (read-after-write) still in store buffer not yet committed")},
  {"RAW_hit_st_q", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("Loads delayed by a previous store (read-after-write) committed but in store queue not yet written to L2D")},

  {"St_q_tag_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {"St_hit_L2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("Stores whose cacheline being updated was in local L2D")},
  {"St_hit_L3", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("Stores whose cacheline being updated was in local L3")},

  {"DC_miss_L2_L3_hit", NULL, REGNO_ANY, NULL, PRELOAD (20, 4), 0, ABST_NONE, STXT ("Loads that speculatively hit local L2D or L3")},
  {"DC_miss_local_hit", NULL, REGNO_ANY, NULL, PRELOAD (500, 4), 0, ABST_NONE, STXT ("Loads that speculatively hit local memory")},
  {"DC_miss_remote_L3_hit", NULL, REGNO_ANY, NULL, PRELOAD (800, 4), 0, ABST_NONE, STXT ("Loads that speculatively hit remote cache or remote memory")},
  {"DC_miss", NULL, REGNO_ANY, NULL, PRELOAD (20, 4), 0, ABST_NONE, STXT ("Loads that speculatively missed L1D")},

  {"L2_pipe_stall", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {"Br_dir_mispred", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("Branch instructions completed whose direction was mispredicted")},
  {"Br_trg_mispred", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("Branch instructions completed whose target was mispredicted")},
  {"Br_mispred", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("Branch instructions completed whose direction or target was mispredicted")},

  {"Cycles_user", NULL, REGNO_ANY, NULL, PRELOAD (1, 4), 1, ABST_NONE, STXT ("Cycles hardware thread is active in specified mode(s)")},
  //
  {"Commit_0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles no uop commits from this hardware thread")},
  {"Commit_0_all", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles no uop commits from any hardware thread on this core")},
  {"Commit_1", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles 1 uop commits from this hardware thread")},
  {"Commit_2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles 2 uops commit from this hardware thread")},
  {"Commit_1_or_2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles 1 or 2 uops commit from this hardware thread")},

  /* additional (hidden) aliases, for convenience */
  {"cycles0", "Cycles_user", 0, NULL, PRELOADS_8, 1, ABST_NONE},
  {"cycles1", "Cycles_user", 1, NULL, PRELOADS_8, 1, ABST_NONE},
  {"insts0", "Instr_all", 0, NULL, PRELOADS_8, 0, ABST_NONE},
  {"insts1", "Instr_all", 1, NULL, PRELOADS_8, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

static Hwcentry sparc_t5_m6[] = {
  // Identical to sparc_t4 except for: l3m_spec
  // when updating this table, also update sparc_t4[]
  // obsolete aliases marked with REGNO_INVALID (allows reading of older experiments)
  {"l2l3dh", "DC_miss_L2_L3_hit_nospec", REGNO_INVALID, STXT ("L2 or L3 D-cache Hits"), PRELOADS_6, 0, ABST_EXACT}, // undercounts due to thread-hog issue
  {"l3m", "DC_miss_remote_L3_hit_nospec~emask=0x6", REGNO_INVALID, STXT ("L3 D-cache Misses"), PRELOADS_5, 0, ABST_EXACT}, // undercounts due to thread-hog issue
  {"lmh", "DC_miss_local_hit_nospec", REGNO_INVALID, STXT ("Local Mem. Hits"), PRELOADS_5, 0, ABST_EXACT}, // undercounts due to thread-hog issue
  {"rmh", "DC_miss_remote_L3_hit_nospec", REGNO_INVALID, STXT ("Remote Mem. Hits"), PRELOADS_5, 0, ABST_EXACT}, // undercounts due to thread-hog issue
  {"pqs", "PQ_tag_wait", REGNO_INVALID, STXT ("Pick Queue Stalls"), PRELOADS_7, 1, ABST_NONE}, // old alias name
  {"raw_stb", "RAW_hit_st_buf", REGNO_INVALID, STXT ("RAW Hazard in Store Buffer"), PRELOADS_55, 0, ABST_NONE}, // 11@full hit, 60@partial hit (in future, combine w/st_q)
  {"raw_stq", "RAW_hit_st_q", REGNO_INVALID, STXT ("RAW Hazard in Store Queue"), PRELOADS_55, 0, ABST_NONE}, // 11@full hit, 60@partial hit (in future, combine w/st_buf)
  {"sel_stalls", "Sel_0_ready", REGNO_INVALID, STXT ("Stalls Another Thread Selected"), PRELOADS_7, 1, ABST_NONE},
  {"icm", "IC_miss", REGNO_INVALID, STXT ("L1 I-Cache Misses"), PRELOADS_55, 0, ABST_NONE}, // 20@ l2/l3 hit (guess)
  {"icm_stalls", "IC_miss", REGNO_INVALID, STXT ("L1 I-Cache Miss Est Stalls"), PRELOADS_55, 25, ABST_NONE}, // 25@ l2-20/l3-50

  // current aliases
  SPARC_CYCLES
  {"cycles", "Cycles_user", REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_75, 1, ABST_NONE},
  {"insts", "Instr_all", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_75, 0, ABST_NONE},
  {"c_stalls", "Commit_0", REGNO_ANY, STXT ("Stall Cycles"), PRELOADS_7, 1, ABST_NONE},

  {"loads", "Instr_ld", REGNO_ANY, STXT ("Load Instructions"), PRELOADS_7, 0, ABST_EXACT},
  {"stores", "Instr_st", REGNO_ANY, STXT ("Store Instructions"), PRELOADS_7, 0, ABST_EXACT},
  {"dcm", "DC_miss_nospec", REGNO_ANY, STXT ("L1 D-cache Misses"), PRELOADS_65, 0, ABST_EXACT},
  //  {"l3m_spec",	"DC_miss_local_hit~emask=0x6",		REGNO_ANY, STXT("L3 D-cache Speculative Misses"),PRELOADS_5,0, ABST_NONE, STXT("Loads that speculatively missed local L3")}, // T4 encoding (430 lm, 690 rm) ~5 misses overlap on t5/pico_ile
  {"l3m_spec", "DC_miss_local_hit~emask=0x30", REGNO_ANY, STXT ("L3 D-cache Speculative Misses"), PRELOADS_5, 0, ABST_NONE, STXT ("Loads that speculatively missed local L3")}, // T5/M6 encoding (430 lm, 690 rm) ~5 misses overlap on t5/pico_ile
  {"lmh_spec", "DC_miss_local_hit", REGNO_ANY, STXT ("Local Mem Speculative Hits"), PRELOADS_5, 0, ABST_NONE},
  {"rmh_spec", "DC_miss_remote_L3_hit", REGNO_ANY, STXT ("Remote Mem Speculative Hits"), PRELOADS_5, 0, ABST_NONE},
  //
  {"dtlbm", "DTLB_miss_asynch", REGNO_ANY, STXT ("DTLB Misses"), PRELOADS_55, 0, ABST_NONE}, // 10@l1 hit, 24@l2 hit, 60@l3 hit, 500@l3 miss, 5000@trap  0.001 events/cycle
  {"dtlb_hwtw_stalls", "DTLB_HWTW_all", REGNO_ANY, STXT ("DTLB HWTW Est Stalls"), PRELOADS_55, 25, ABST_NONE, STXT ("Estimated time stalled on a DTLB miss requiring a HW tablewalk")}, // l2-20, l3-50
  {"dtlb_trap_stalls", "DTLB_fill_trap", REGNO_ANY, STXT ("DTLB Trap Est Stalls"), PRELOADS_35, 5000, ABST_NONE, STXT ("Estimated time stalled on a DTLB miss with HW tablewalk unsuccessful")}, // 5000@trap
  {"rawhaz", "RAW_hit_st_q~emask=0xf", REGNO_ANY, STXT ("Read-after-write Hazards"), PRELOADS_55, 0, ABST_NONE, STXT ("Loads delayed by a previous store (read-after-write hazards)")},
  {"br_msp_stalls", "Br_mispred", REGNO_ANY, STXT ("Branch Mispredict Stalls"), PRELOADS_6, 24, ABST_NONE, STXT ("Estimated time stalled on Branch mispredictions")}, // 24@miss, %5 of branches is bad
  {"br_msp", "Br_mispred", REGNO_ANY, STXT ("Branch Mispredict"), PRELOADS_6, 0, ABST_NONE}, // 24@miss, %5 of branches is bad
  {"br_tkn", "Br_taken", REGNO_ANY, STXT ("Branch Taken"), PRELOADS_7, 0, ABST_NONE}, // 2 cycles minimum
  {"br_ins", "Branches", REGNO_ANY, STXT ("Branch Instructions"), PRELOADS_7, 0, ABST_NONE}, // 24@miss, %5 of branches is bad
  {"fgu", "Instr_FGU_crypto", REGNO_ANY, STXT ("FP/VIS/Crypto Instructions"), PRELOADS_7, 0, ABST_NONE}, // 1 cycle/event

  /* explicit definitions of (hidden) entries for proper counters */
  /* Counters that can be time converted, support memspace, or have a short_desc need to be in this table */

  {"Sel_pipe_drain_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles a hardware thread stalls at Select waiting with correct instructions when pipeline has to drain after branch misprediction")},
  {"Sel_0_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles a hardware thread stalls at Select waiting for various conditions to be resolved")},
  {"Sel_0_ready", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles a hardware thread was ready to have its instructions selected but another hardware thread was selected instead")},
  {"Sel_1", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles that only 1 instruction or uop was selected")},
  {"Sel_2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles that 2 instructions or uops were selected")},

  {"Pick_0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Pick_1", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Pick_2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Pick_3", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Pick_any", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {"Branches", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("Control transfer instructions completed, excluding trap-related transfers")},
  {"Instr_FGU_crypto", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("FP and VIS instructions completed by the Floating Point and Graphics Unit")},
  {"Instr_ld", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT, STXT ("Load instructions completed")},
  {"Instr_st", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT, STXT ("Store instructions completed")},
  {"SPR_ring_ops", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT, STXT ("Specialized instructions that require internal use of SPR ring completed")},
  {"Instr_other", NULL, REGNO_ANY, NULL, PRELOAD (2, 4), 0, ABST_NONE, STXT ("Basic arithmetic and logical instructions completed")},
  {"Instr_all", NULL, REGNO_ANY, NULL, PRELOAD (1, 4), 0, ABST_NONE, STXT ("Total instructions completed")},

  {"Br_taken", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("Branch instructions taken and completed")},
  {"Sw_count_intr", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("SW Count instructions completed")},
  {"Atomics", NULL, REGNO_ANY, NULL, PRELOAD (20, 4), 0, ABST_EXACT, STXT ("Atomic instructions, including CASA/XA, completed")},
  {"SW_prefetch", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT, STXT ("PREFETCH and PREFETCHA instructions completed")},
  {"Block_ld_st", NULL, REGNO_ANY, NULL, PRELOAD (20, 4), 0, ABST_EXACT, STXT ("Block load/store instructions completed")},

  {"BTC_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("Branches delayed a few extra cycles because branch target not found in Branch Target Cache")},

  {"ITLB_fill_8KB", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk successfully loaded translation for 8K page")},
  {"ITLB_fill_64KB", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk successfully loaded translation for 64K page")},
  {"ITLB_fill_4MB", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk successfully loaded translation for 4M page")},
  {"ITLB_fill_256MB", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk successfully loaded translation for 256M page")},
  {"ITLB_fill_2GB", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk successfully loaded translation for 2G or 16G page")},
  {"ITLB_fill_trap", NULL, REGNO_ANY, NULL, PRELOAD (1000, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk unsuccessful")},
  {"ITLB_miss_asynch", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk search done")},

  {"Fetch_0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Fetch_0_all", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {"Instr_buffer_full", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {"PQ_tag_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"ROB_tag_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"LB_tag_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"ROB_LB_tag_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"SB_tag_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"ROB_SB_tag_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"LB_SB_tag_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"ROB_LB_SB_tag_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"DTLB_miss_tag_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {"ITLB_HWTW_L2_hit", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk hit local L2D")},
  {"ITLB_HWTW_L3_hit", NULL, REGNO_ANY, NULL, PRELOAD (80, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk hit local L3 or neighbor L2D")},
  {"ITLB_HWTW_L3_miss", NULL, REGNO_ANY, NULL, PRELOAD (800, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk missed all local caches")},
  {"DTLB_HWTW_L2_hit", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk hit local L2D")},
  {"DTLB_HWTW_L3_hit", NULL, REGNO_ANY, NULL, PRELOAD (80, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk hit local L3 or neighbor L2D")},
  {"DTLB_HWTW_L3_miss", NULL, REGNO_ANY, NULL, PRELOAD (800, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk missed all local caches")},
  {"DTLB_HWTW_all", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss requiring HW tablewalk")},

  {"DC_miss_L2_L3_hit_nospec", NULL, REGNO_ANY, NULL, PRELOAD (25, 4), 0, ABST_EXACT},
  {"DC_miss_local_hit_nospec", NULL, REGNO_ANY, NULL, PRELOAD (500, 4), 0, ABST_EXACT},
  {"DC_miss_remote_L3_hit_nospec", NULL, REGNO_ANY, NULL, PRELOAD (800, 4), 0, ABST_EXACT},
  {"DC_miss_nospec", NULL, REGNO_ANY, NULL, PRELOAD (25, 4), 0, ABST_EXACT, STXT ("Loads that missed local L1D")},

  {"DTLB_fill_8KB", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk successfully loaded translation for 8K page")},
  {"DTLB_fill_64KB", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk successfully loaded translation for 64K page")},
  {"DTLB_fill_4MB", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk successfully loaded translation for 4M page")},
  {"DTLB_fill_256MB", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk successfully loaded translation for 256M page")},
  {"DTLB_fill_2GB", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk successfully loaded translation for 2G or 16G page")},
  {"DTLB_fill_trap", NULL, REGNO_ANY, NULL, PRELOAD (800, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk unsuccessful")},
  {"DTLB_miss_asynch", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk search done")},
  {"RAW_hit_st_buf", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("Loads delayed by a previous store (read-after-write) still in store buffer not yet committed")},
  {"RAW_hit_st_q", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("Loads delayed by a previous store (read-after-write) committed but in store queue not yet written to L2D")},

  {"St_q_tag_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {"St_hit_L2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("Stores whose cacheline being updated was in local L2D")},
  {"St_hit_L3", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("Stores whose cacheline being updated was in local L3")},

  {"DC_miss_L2_L3_hit", NULL, REGNO_ANY, NULL, PRELOAD (20, 4), 0, ABST_NONE, STXT ("Loads that speculatively hit local L2D or L3")},
  {"DC_miss_local_hit", NULL, REGNO_ANY, NULL, PRELOAD (500, 4), 0, ABST_NONE, STXT ("Loads that speculatively hit local memory")},
  {"DC_miss_remote_L3_hit", NULL, REGNO_ANY, NULL, PRELOAD (800, 4), 0, ABST_NONE, STXT ("Loads that speculatively hit remote cache or remote memory")},
  {"DC_miss", NULL, REGNO_ANY, NULL, PRELOAD (20, 4), 0, ABST_NONE, STXT ("Loads that speculatively missed L1D")},

  {"L2_pipe_stall", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {"Br_dir_mispred", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("Branch instructions completed whose direction was mispredicted")},
  {"Br_trg_mispred", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("Branch instructions completed whose target was mispredicted")},
  {"Br_mispred", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("Branch instructions completed whose direction or target was mispredicted")},

  {"Cycles_user", NULL, REGNO_ANY, NULL, PRELOAD (1, 4), 1, ABST_NONE, STXT ("Cycles hardware thread is active in specified mode(s)")},
  //
  {"Commit_0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles no uop commits from this hardware thread")},
  {"Commit_0_all", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles no uop commits from any hardware thread on this core")},
  {"Commit_1", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles 1 uop commits from this hardware thread")},
  {"Commit_2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles 2 uops commit from this hardware thread")},
  {"Commit_1_or_2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles 1 or 2 uops commit from this hardware thread")},

  /* additional (hidden) aliases, for convenience */
  {"cycles0", "Cycles_user", 0, NULL, PRELOADS_8, 1, ABST_NONE},
  {"cycles1", "Cycles_user", 1, NULL, PRELOADS_8, 1, ABST_NONE},
  {"insts0", "Instr_all", 0, NULL, PRELOADS_8, 0, ABST_NONE},
  {"insts1", "Instr_all", 1, NULL, PRELOADS_8, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

static Hwcentry sparc_m7[] = {
  // obsolete aliases marked with REGNO_INVALID (allows reading of older experiments)
  {"icm", "IC_miss_commit", REGNO_INVALID, STXT ("L1 I-Cache Misses"), PRELOADS_6, 0, ABST_EXACT},
  {"raw_stb", "RAW_hit_st_buf", REGNO_INVALID, STXT ("RAW Hazard in Store Buffer"), PRELOADS_55, 0, ABST_NONE},
  {"raw_stq", "RAW_hit_st_q", REGNO_INVALID, STXT ("RAW Hazard in Store Queue"), PRELOADS_55, 0, ABST_NONE},
  {"pqs", "PQ_tag_wait_cyc", REGNO_INVALID, STXT ("Pick Queue Stalls"), PRELOADS_7, 1, ABST_NONE},
  {"sel_stalls", "Sel_0_ready_cyc", REGNO_INVALID, STXT ("Stalls Another Thread Selected"), PRELOADS_7, 1, ABST_NONE},

  // current aliases
  SPARC_CYCLES
  {"cycles", "Cycles_user", REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_75, 1, ABST_NONE},
  {"insts", "Instr_all", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_75, 0, ABST_NONE},
  {"c_stalls", "Commit_0_cyc", REGNO_ANY, STXT ("Stall Cycles"), PRELOADS_7, 1, ABST_NONE},

  {"loads", "Instr_ld", REGNO_ANY, STXT ("Load Instructions"), PRELOADS_7, 0, ABST_EXACT},
  {"stores", "Instr_st", REGNO_ANY, STXT ("Store Instructions"), PRELOADS_6, 0, ABST_EXACT},
  {"dcm", "DC_miss_commit", REGNO_ANY, STXT ("L1 D-cache Misses"), PRELOADS_6, 0, ABST_EXACT},

  {"l3m_spec", "DC_miss_L3_miss", REGNO_ANY, STXT ("L3 D-cache Speculative Misses"), PRELOADS_5, 0, ABST_NONE},
  {"lmh_spec", "DC_miss_local_mem_hit", REGNO_ANY, STXT ("Local Mem Speculative Hits"), PRELOADS_5, 0, ABST_NONE},
  {"rmh_spec", "DC_miss_remote_mem_hit", REGNO_ANY, STXT ("Remote Mem Speculative Hits"), PRELOADS_5, 0, ABST_NONE},
  //
  {"dtlbm", "DTLB_HWTW_search", REGNO_ANY, STXT ("DTLB Misses"), PRELOADS_55, 0, ABST_NONE}, // 10@l1 hit, 24@l2 hit, 60@l3 hit, 500@l3 miss, 5000@trap  0.001 events/cycle
  {"dtlb_hwtw_stalls", "DTLB_HWTW_ref", REGNO_ANY, STXT ("DTLB HWTW Est Stalls"), PRELOADS_55, 25, ABST_NONE, STXT ("Estimated time stalled on a DTLB miss requiring a HW tablewalk")}, // l2-20, l3-50
  {"dtlb_trap_stalls", "DTLB_HWTW_miss_trap", REGNO_ANY, STXT ("DTLB Trap Est Stalls"), PRELOADS_35, 5000, ABST_NONE, STXT ("Estimated time stalled on a DTLB miss with HW tablewalk unsuccessful")}, // 5000@trap
  {"rawhaz", "RAW_hit_st_q~emask=0xf", REGNO_ANY, STXT ("Read-after-write Hazards"), PRELOADS_55, 0, ABST_NONE, STXT ("Loads delayed by a previous store (read-after-write hazards)")},
  {"br_msp_stalls", "Br_mispred", REGNO_ANY, STXT ("Branch Mispredict Stalls"), PRELOADS_6, 24, ABST_NONE, STXT ("Estimated time stalled on Branch mispredictions")}, // 24@miss, %5 of branches is bad
  {"br_msp", "Br_mispred", REGNO_ANY, STXT ("Branch Mispredict"), PRELOADS_6, 0, ABST_NONE},
  {"br_tkn", "Br_taken", REGNO_ANY, STXT ("Branch Taken"), PRELOADS_7, 0, ABST_NONE},
  {"br_ins", "Branches", REGNO_ANY, STXT ("Branch Instructions"), PRELOADS_7, 0, ABST_NONE},
  {"fgu", "Instr_FGU_crypto", REGNO_ANY, STXT ("FP/VIS/Crypto Instructions"), PRELOADS_7, 0, ABST_NONE},
  {"spill_fill", "Flush_arch_exception", REGNO_ANY, STXT ("Reg Window Spill/Fill Est Stalls"), PRELOAD (100, 4), 80, ABST_NONE, STXT ("Estimated time stalled on flushing pipeline due to register window spill/fill")},

  /* explicit definitions of (hidden) entries for proper counters */
  /*  Counters that can be time converted, support memspace, or have a short_desc need to be in this table */
  {"Sel_pipe_drain_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles a hardware thread stalls at Select waiting with correct instructions when pipeline has to drain after branch misprediction")},
  {"Sel_0_wait_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles a hardware thread stalls at Select waiting for various conditions to be resolved")},
  {"Sel_0_ready_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles a hardware thread was ready to have its instructions selected but another hardware thread was selected instead")},
  {"Sel_1_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles that only 1 instruction or uop was selected")},
  {"Sel_2_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles that 2 instructions or uops were selected")},

  {"Pick_0_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Pick_1_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Pick_2_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Pick_3_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Pick_any_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {"Branches", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("Control transfer instructions completed, excluding trap-related transfers")},
  {"Instr_FGU_crypto", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("FP and VIS instructions completed by the Floating Point and Graphics Unit")},
  {"Instr_ld", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT, STXT ("Load instructions completed")},
  {"Instr_st", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT, STXT ("Store instructions completed")},
  {"Instr_SPR_ring_ops", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT, STXT ("Specialized instructions that require internal use of SPR ring completed")},
  {"Instr_other", NULL, REGNO_ANY, NULL, PRELOAD (2, 4), 0, ABST_NONE, STXT ("Basic arithmetic and logical instructions completed")},
  {"Instr_all", NULL, REGNO_ANY, NULL, PRELOAD (1, 4), 0, ABST_NONE, STXT ("Total instructions completed")},

  {"Br_taken", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("Branch instructions taken and completed")},
  {"Instr_SW_count", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("SW Count instructions completed")},
  {"Instr_atomic", NULL, REGNO_ANY, NULL, PRELOAD (20, 4), 0, ABST_EXACT, STXT ("Atomic instructions, including CASA/XA, completed")},
  {"Instr_SW_prefetch", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT, STXT ("PREFETCH and PREFETCHA instructions completed")},
  {"Instr_block_ld_st", NULL, REGNO_ANY, NULL, PRELOAD (20, 4), 0, ABST_EXACT, STXT ("Block load/store instructions completed")},

  {"Br_BTC_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("Branches delayed a few extra cycles because branch target not found in Branch Target Cache")},

  {"ITLB_HWTW_hit_8K", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk successfully loaded translation for 8K page")},
  {"ITLB_HWTW_hit_64K", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk successfully loaded translation for 64K page")},
  {"ITLB_HWTW_hit_4M", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk successfully loaded translation for 4M page")},
  {"ITLB_HWTW_hit_256M", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk successfully loaded translation for 256M page")},
  {"ITLB_HWTW_hit_2G_16G", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk successfully loaded translation for 2G or 16G page")},
  {"ITLB_HWTW_miss_trap", NULL, REGNO_ANY, NULL, PRELOAD (1000, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk unsuccessful")},
  {"ITLB_HWTW_search", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk search done")},

  {"Fetch_0_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Fetch_0_all_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {"Instr_buffer_full_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {"PQ_tag_wait_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"ROB_tag_wait_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"LB_tag_wait_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"SB_tag_wait_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"ROB_LB_tag_wait_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"ROB_SB_tag_wait_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"LB_SB_tag_wait_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"ROB_LB_SB_tag_wait_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"DTLB_miss_tag_wait_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {"ITLB_HWTW_L2_hit", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk hit local L2D")},
  {"ITLB_HWTW_L3_hit", NULL, REGNO_ANY, NULL, PRELOAD (80, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk hit local L3 or neighbor L2D")},
  {"ITLB_HWTW_L3_miss", NULL, REGNO_ANY, NULL, PRELOAD (800, 4), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk missed all local caches")},
  {"DTLB_HWTW_L2_hit", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk hit local L2D")},
  {"DTLB_HWTW_L3_hit", NULL, REGNO_ANY, NULL, PRELOAD (80, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk hit local L3 or neighbor L2D")},
  {"DTLB_HWTW_L3_miss", NULL, REGNO_ANY, NULL, PRELOAD (800, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk missed all local caches")},
  {"DTLB_HWTW_ref", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss requiring HW tablewalk")},

  {"DC_miss_L2_L3_hit_commit", NULL, REGNO_ANY, NULL, PRELOAD (25, 4), 0, ABST_EXACT},
  {"DC_miss_nbr_scc_hit_commit", NULL, REGNO_ANY, NULL, PRELOAD (500, 4), 0, ABST_EXACT},
  {"DC_miss_nbr_scc_miss_commit", NULL, REGNO_ANY, NULL, PRELOAD (800, 4), 0, ABST_EXACT},
  {"DC_miss_commit", NULL, REGNO_ANY, NULL, PRELOAD (25, 4), 0, ABST_EXACT, STXT ("Loads that missed local L1D")},

  {"DTLB_HWTW_hit_8K", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk successfully loaded translation for 8K page")},
  {"DTLB_HWTW_hit_64K", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk successfully loaded translation for 64K page")},
  {"DTLB_HWTW_hit_4M", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk successfully loaded translation for 4M page")},
  {"DTLB_HWTW_hit_256M", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk successfully loaded translation for 256M page")},
  {"DTLB_HWTW_hit_2G_16G", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk successfully loaded translation for 2G or 16G page")},
  {"DTLB_HWTW_miss_trap", NULL, REGNO_ANY, NULL, PRELOAD (800, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk unsuccessful")},
  {"DTLB_HWTW_search", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk search done")},
  {"RAW_hit_st_buf", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("Loads delayed by a previous store (read-after-write) still in store buffer not yet committed")},
  {"RAW_hit_st_q", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("Loads delayed by a previous store (read-after-write) committed but in store queue not yet written to L2D")},

  {"St_q_tag_wait_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {"St_L2_hit", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("Stores whose cacheline being updated was in local L2D")},
  {"St_L3_hit", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("Stores whose cacheline being updated was in local L3")},

  {"DC_hit", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("Loads that speculatively hit local L1D")},
  {"DC_miss_L2_hit", NULL, REGNO_ANY, NULL, PRELOAD (20, 4), 0, ABST_NONE, STXT ("Loads that speculatively hit local L2D")},
  {"DC_miss_L3_hit", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("Loads that speculatively hit local L3")},
  {"DC_miss_nbr_L2_hit", NULL, REGNO_ANY, NULL, PRELOAD (100, 4), 0, ABST_NONE, STXT ("Loads that speculatively hit neighbor L2D via local L3")},
  {"DC_miss_nbr_scc_hit", NULL, REGNO_ANY, NULL, PRELOAD (100, 4), 0, ABST_NONE, STXT ("Loads that speculatively hit neighbor L3 on same socket")},
  {"DC_miss_nbr_scc_miss", NULL, REGNO_ANY, NULL, PRELOAD (400, 4), 0, ABST_NONE, STXT ("Loads that speculatively missed all caches on same socket")},
  {"DC_miss", NULL, REGNO_ANY, NULL, PRELOAD (10, 4), 0, ABST_NONE, STXT ("Loads that speculatively missed local L1D")},
  {"DC_miss_L2_miss", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("Loads that speculatively missed local L2D")},
  {"DC_miss_L3_miss", NULL, REGNO_ANY, NULL, PRELOAD (200, 4), 0, ABST_NONE, STXT ("Loads that speculatively missed local L3")},

  {"DC_miss_remote_scc_hit", NULL, REGNO_ANY, NULL, PRELOAD (800, 4), 0, ABST_NONE, STXT ("Loads that speculatively hit remote cache on different socket")},
  {"DC_miss_local_mem_hit", NULL, REGNO_ANY, NULL, PRELOAD (500, 4), 0, ABST_NONE, STXT ("Loads that speculatively hit local memory")},
  {"DC_miss_remote_mem_hit", NULL, REGNO_ANY, NULL, PRELOAD (1000, 4), 0, ABST_NONE, STXT ("Loads that speculatively hit remote memory")},
  {"Br_dir_mispred", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("Branch instructions completed whose direction was mispredicted")},
  {"Br_tgt_mispred", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("Branch instructions completed whose target was mispredicted")},
  {"Br_mispred", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("Branch instructions completed whose direction or target was mispredicted")},

  {"Cycles_user", NULL, REGNO_ANY, NULL, PRELOAD (1, 4), 1, ABST_NONE, STXT ("Cycles hardware thread is active in specified mode(s)")},

  {"Flush_L3_miss", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("Pipeline flushes due to a load that misses L3 when more than 1 hardware thread is active on the core")},
  {"Flush_br_mispred", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("Pipeline flushes due to a branch misprediction")},
  {"Flush_arch_exception", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("Pipeline flushes due to SPARC architecture exceptions and trap entry/return")},
  {"Flush_other", NULL, REGNO_ANY, NULL, PRELOAD (40, 4), 0, ABST_NONE, STXT ("Pipeline flushes due to hardware thread state change to/from halted/paused state")},
  //
  {"Commit_0_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles no uop commits from this hardware thread")},
  {"Commit_0_all_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles no uop commits from any hardware thread on this core")},
  {"Commit_1_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles 1 uop commits from this hardware thread")},
  {"Commit_2_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles 2 uops commit from this hardware thread")},
  {"Commit_1_or_2_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles 1 or 2 uops commit from this hardware thread")},


  /* additional (hidden) aliases, for convenience */
  {"cycles0", "Cycles_user", 0, NULL, PRELOADS_8, 1, ABST_NONE},
  {"cycles1", "Cycles_user", 1, NULL, PRELOADS_8, 1, ABST_NONE},
  {"insts0", "Instr_all", 0, NULL, PRELOADS_8, 0, ABST_NONE},
  {"insts1", "Instr_all", 1, NULL, PRELOADS_8, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

static Hwcentry sparc_m8[] = {
  // current aliases
  SPARC_CYCLES
  {"cycles", "Cycles_user", REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_75, 1, ABST_NONE},
  {"insts", "Instr_all", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_75, 0, ABST_NONE},
  {"c_stalls", "Commit_0_cyc", 3, STXT ("Stall Cycles"), PRELOADS_7, 1, ABST_NONE}, // 22825776: limit to reg 3
  {"Sel_0_wait_cyc", "Sel_0_cyc~emask=0x3f", REGNO_ANY, STXT ("Select Stall Cycles"), PRELOADS_7, 1, ABST_NONE, STXT ("Cycles a hardware thread stalls at Select waiting for various conditions to be resolved that prevent it being selected")},

  {"loads", "Instr_ld", REGNO_ANY, STXT ("Load Instructions"), PRELOADS_7, 0, ABST_EXACT},
  {"stores", "Instr_st", REGNO_ANY, STXT ("Store Instructions"), PRELOADS_6, 0, ABST_EXACT},
  {"dcm", "DC_miss_commit", REGNO_ANY, STXT ("L1 D-cache Misses"), PRELOADS_6, 0, ABST_EXACT},

  {"lmh_spec", "DC_miss_local_mem_hit", REGNO_ANY, STXT ("Local Mem Speculative Hits"), PRELOADS_5, 0, ABST_NONE},
  {"rmh_spec", "DC_miss_remote_mem_hit", REGNO_ANY, STXT ("Remote Mem Speculative Hits"), PRELOADS_5, 0, ABST_NONE},

  {"dtlbm", "DTLB_HWTW", REGNO_ANY, STXT ("DTLB Misses"), PRELOAD (40, 5), 0, ABST_NONE}, // 10@l1 hit, 24@l2 hit, 60@l3 hit, 500@l3 miss, 5000@trap  0.001 events/cycle
  {"dtlb_hwtw_stalls", "DTLB_HWTW", REGNO_ANY, STXT ("DTLB HWTW Est Stalls"), PRELOAD (40, 5), 25, ABST_NONE, STXT ("Estimated time stalled on a DTLB miss requiring a HW tablewalk")}, // l2-20, l3-50
  {"dtlb_trap_stalls", "DTLB_HWTW_miss_trap", REGNO_ANY, STXT ("DTLB Trap Est Stalls"), PRELOAD (800, 5), 5000, ABST_NONE, STXT ("Estimated time stalled on a DTLB miss with HW tablewalk unsuccessful")}, // 5000@trap
  {"rawhaz", "RAW_hit", REGNO_ANY, STXT ("Read-after-write Hazards"), PRELOAD (40, 5), 0, ABST_NONE},
  {"br_msp_stalls", "Br_mispred", REGNO_ANY, STXT ("Branch Mispredict Stalls"), PRELOAD (40, 5), 24, ABST_NONE, STXT ("Estimated time stalled on Branch mispredictions")}, // 24@miss, %5 of branches is bad
  {"br_msp", "Br_mispred", REGNO_ANY, STXT ("Branch Mispredict"), PRELOAD (40, 5), 0, ABST_NONE},
  {"br_tkn", "Br_taken", REGNO_ANY, STXT ("Branch Taken"), PRELOADS_7, 0, ABST_NONE},
  {"br_ins", "Branches", REGNO_ANY, STXT ("Branch Instructions"), PRELOADS_7, 0, ABST_NONE},
  {"fgu", "Instr_FGU_crypto", REGNO_ANY, STXT ("FP/VIS/Crypto Instructions"), PRELOADS_7, 0, ABST_NONE},
  {"spill_fill", "Flush_spill_fill", REGNO_ANY, STXT ("Reg Window Spill/Fill Est Stalls"), PRELOAD (100, 5), 80, ABST_NONE, STXT ("Estimated time stalled on flushing pipeline due to register window spill/fill")},

  /* explicit definitions of (hidden) entries for proper counters */
  /*  Counters that can be time converted, support memspace, or have a short_desc need to be in this table */
  //0x01
  {"Fetch_stall_IFU_reset_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Fetch_stall_IC_miss_MB_full_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Fetch_stall_IC_miss_MB_avail_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Fetch_stall_IC_miss_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Fetch_stall_ITLB_miss_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Fetch_stall_SEL_buf_full_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Fetch_ready_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Fetch_0_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Fetch_0_all_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  //0x02
  {"Fetch_1_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Fetch_2_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Fetch_3_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Fetch_4_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Fetch_5_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Fetch_6_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Fetch_7_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Fetch_8_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Fetch_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  //0x07
  {"ITLB_HWTW_hit_8K", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk successfully loaded translation for 8K page")},
  {"ITLB_HWTW_hit_64K", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk successfully loaded translation for 64K page")},
  {"ITLB_HWTW_hit_4M", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk successfully loaded translation for 4M page")},
  {"ITLB_HWTW_hit_256M", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk successfully loaded translation for 256M page")},
  {"ITLB_HWTW_hit_16G", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk successfully loaded translation for 16G page")},
  {"ITLB_HWTW_hit_1T", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk successfully loaded translation for 1T page")},
  //  { "ITLB_HWTW_miss_RA2PAC",		0x0740, 0xf07ff },
  //  { "ITLB_HWTW_miss_not_RA2PAC",		0x0780, 0xf07ff },
  {"ITLB_HWTW_miss_trap", NULL, REGNO_ANY, NULL, PRELOAD (1000, 5), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk unsuccessful")},
  {"ITLB_HWTW", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk search done")},
  //0x08
  {"Br_BTC_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("Branches delayed a few extra cycles because branch target not found in Branch Target Cache")},
  //0x09
  {"Sel_0_no_instr_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles a hardware thread stalls at Select because no instructions are available")},
  {"Sel_0_pipe_drain_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles a hardware thread stalls at Select waiting with correct instructions when pipeline has to drain after branch misprediction")},
  {"Sel_0_postsync_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles a hardware thread stalls at Select waiting for prior instructions to commit")},
  {"Sel_0_presync_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles a hardware thread stalls at Select with instruction that cannot decode until prior instructions have committed")},
  {"Sel_0_thread_hog_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles a hardware thread stalls at Select to prevent strand monopolizing resources")},
  {"Sel_0_tag_stall_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles a hardware thread stalls at Select because no required tags are available")},
  {"Sel_0_ready_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles a hardware thread was ready to have its instructions selected but another hardware thread was selected instead")},
  {"Sel_0_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles a hardware thread is not selected")},
  // No direct equivalent Sel_1/2_cyc. Nearest is Decode_uop, which increments by 0-4 each cycle according to how many uops were decoded.
  //0x13
  {"ITLB_HWTW_L2_hit", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk hit local L2D")},
  {"ITLB_HWTW_L3_hit", NULL, REGNO_ANY, NULL, PRELOAD (80, 5), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk hit local L3 or neighbor L2D")},
  {"ITLB_HWTW_L3_miss", NULL, REGNO_ANY, NULL, PRELOAD (800, 5), 0, ABST_NONE, STXT ("ITLB miss and HW tablewalk missed all local caches")},
  {"DTLB_HWTW_L2_hit", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk hit local L2D")},
  {"DTLB_HWTW_L3_hit", NULL, REGNO_ANY, NULL, PRELOAD (80, 5), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk hit local L3 or neighbor L2D")},
  {"DTLB_HWTW_L3_miss", NULL, REGNO_ANY, NULL, PRELOAD (800, 5), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk missed all local caches")},
  {"DTLB_HWTW_ref", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("DTLB miss requiring HW tablewalk")},
  //0x0E
  {"Instr_FGU_crypto", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("FP and VIS instructions completed by the Floating Point and Graphics Unit")},
  {"Instr_ld", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT, STXT ("Load instructions completed")},
  {"Instr_st", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT, STXT ("Store instructions completed")},
  {"Instr_block_ld_st", NULL, REGNO_ANY, NULL, PRELOAD (20, 5), 0, ABST_EXACT, STXT ("Block load/store instructions completed")},
  {"Instr_SPR_ring_ops", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT, STXT ("Specialized instructions that require internal use of SPR ring completed")},
  {"Instr_atomic", NULL, REGNO_ANY, NULL, PRELOAD (20, 5), 0, ABST_EXACT, STXT ("Atomic instructions, including CASA/XA, completed")},
  {"Instr_SW_prefetch", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT, STXT ("PREFETCH and PREFETCHA instructions completed")},
  {"Instr_other", NULL, REGNO_ANY, NULL, PRELOAD (2, 5), 0, ABST_NONE, STXT ("Basic arithmetic and logical instructions completed")},
  {"Instr_all", NULL, REGNO_ANY, NULL, PRELOAD (1, 5), 0, ABST_NONE, STXT ("Total instructions completed")},
  //0x0F
  {"Branches", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("Control transfer instructions completed, excluding trap-related transfers")},
  //0x10
  {"Br_taken", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("Branch instructions taken and completed")},
  //0x11
  {"Rename_tag_wait_PQ_1_EXU_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Rename_tag_wait_PQ_0_LSU_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Rename_wait_crypto_diag_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Sel_0_wait_ROB_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Sel_0_wait_WRF_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Sel_0_wait_LB_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Sel_0_wait_SB_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  //0x12
  {"Fetch_stall_BDA_tag_unavail_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Fetch_stall_BTA_tag_unavail_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Fetch_stall_misc_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"Fetch_stall_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"MMU_TTE_buffer_tag_wait_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"MMU_PRQ_pool_tag_wait_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  //0x15
  {"L2I_request_block_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2I_thread_hog_stall_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2I_MB_full_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2I_snoop_eviction", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2I_stall_no_request_credit_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2I_stall_no_response_credit_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  //0x16
  {"Flush_thread_hog", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Pipeline flushes to prevent thread from monopolizing resources")},
  {"Flush_br_mispred", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Pipeline flushes due to a branch misprediction")},
  {"Flush_arch_exception", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Pipeline flushes due to SPARC architecture exceptions and trap entry/return")},
  {"Flush_evil_twin", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Pipeline flushes due to detecting floating point evil twin condition")},
  {"Flush_LSU_trap", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Pipeline flushes to refetch Next-PC")},
  {"Flush_mode_change", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Pipeline flushes due to strand mode change")},
  {"Flush_misalign", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Pipeline flushes due to detecting misaligned load/store requiring transition to misaligned mitigation mode")},
  {"Flush_other", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Pipeline flushes due to hardware thread state change to/from halted/paused state")},
  {"Flush_all", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Pipeline flushes due to any reason")},
  //0x17
  {"Flush_spill_n_normal", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Pipeline flushes due to spill_n_normal exception")},
  {"Flush_spill_n_other", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Pipeline flushes due to spill_n_other exception")},
  {"Flush_fill_n_normal", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Pipeline flushes due to fill_n_normal exception")},
  {"Flush_fill_n_other", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Pipeline flushes due to fill_n_other exception")},
  {"Flush_spill_fill", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Pipeline flushes due to spill/fill exceptions")},
  {"Flush_lost_load", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Pipeline flushes due to speculatively executed load violating memory order")},
  //0x21
  {"Br_dir_mispred", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Branch instructions completed whose direction was mispredicted")},
  {"Br_tgt_mispred", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Branch instructions completed whose target was mispredicted")},
  {"Br_mispred", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Branch instructions completed whose direction or target was mispredicted")},
  //0x23
  {"LSU_st_q_tag_wait_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"LSU_st_q_tag_wait_all_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2D_stall_no_request_credit_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2D_stall_no_response_credit_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  //0x27
  {"DC_miss_L2_hit", NULL, REGNO_ANY, NULL, PRELOAD (20, 5), 0, ABST_NONE, STXT ("Loads that speculatively hit local L2D")},
  {"DC_miss_L3_hit", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Loads that speculatively hit local L3")},
  {"DC_miss_L3_dirty_copyback", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Loads that speculatively hit local L3 but require copyback from L2D within same CPC")},
  {"DC_miss_nbr_L3_hit", NULL, REGNO_ANY, NULL, PRELOAD (100, 5), 0, ABST_NONE, STXT ("Loads that speculatively hit neighbor L3 on same socket")},
  {"DC_miss_remote_L3_hit", NULL, REGNO_ANY, NULL, PRELOAD (400, 5), 0, ABST_NONE, STXT ("Loads that speculatively hit remote cache on different socket")},
  {"DC_miss_local_mem_hit", NULL, REGNO_ANY, NULL, PRELOAD (500, 5), 0, ABST_NONE, STXT ("Loads that speculatively hit local memory")},
  {"DC_miss_remote_mem_hit", NULL, REGNO_ANY, NULL, PRELOAD (1000, 5), 0, ABST_NONE, STXT ("Loads that speculatively hit remote memory")},
  {"DC_miss", NULL, REGNO_ANY, NULL, PRELOAD (10, 5), 0, ABST_NONE, STXT ("Loads that speculatively missed local L1D")},
  //0x28
  {"DC_sec_miss_L2_hit_commit", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT},
  {"DC_miss_L2_hit_commit", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT},
  {"DC_miss_L3_hit_commit", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT},
  {"DC_miss_L3_dirty_copyback_commit", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT},
  {"DC_miss_nbr_L3_hit_commit", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT},
  {"DC_miss_remote_L3_hit_commit", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT},
  {"DC_miss_local_mem_hit_commit", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT},
  {"DC_miss_remote_mem_hit_commit", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_EXACT},
  {"DC_miss_commit", NULL, REGNO_ANY, NULL, PRELOAD (25, 5), 0, ABST_EXACT, STXT ("Loads that missed local L1D")},
  //0x29
  //  {"Store_DC_sec_miss_L2_hit",	 NULL, REGNO_ANY, NULL, PRELOAD_DEF,     0, ABST_NONE,  STXT("")},
  {"Store_L2_hit", NULL, REGNO_ANY, NULL, PRELOAD (20, 5), 0, ABST_NONE, STXT ("Stores whose cacheline being updated was in local L2D")},
  {"Store_L3_hit", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Stores whose cacheline being updated was in local L3")},
  {"Store_nbr_L2_hit", NULL, REGNO_ANY, NULL, PRELOAD (100, 5), 0, ABST_NONE, STXT ("Stores whose cacheline being updated was in neighbor L2 on same socket")},
  {"Store_nbr_L3_hit", NULL, REGNO_ANY, NULL, PRELOAD (100, 5), 0, ABST_NONE, STXT ("Stores whose cacheline being updated was in neighbor L3 on same socket")},
  {"Store_remote_L3_hit", NULL, REGNO_ANY, NULL, PRELOAD (400, 5), 0, ABST_NONE, STXT ("Stores whose cacheline being updated was in remote cache on different socket")},
  {"Store_local_mem_hit", NULL, REGNO_ANY, NULL, PRELOAD (500, 5), 0, ABST_NONE, STXT ("Stores whose cacheline being updated was in local memory")},
  {"Store_remote_mem_hit", NULL, REGNO_ANY, NULL, PRELOAD (1000, 5), 0, ABST_NONE, STXT ("Stores whose cacheline being updated was in remote memory")},
  {"Store_all", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE, STXT ("Stores whose cacheline being updated was observed to be somewhere in the memory hierarchy")},
  //0x2d
  {"RAW_hit_st_buf", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Loads delayed by a previous store (read-after-write) still in store buffer not yet committed")},
  {"RAW_hit_st_q", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Loads delayed by a previous store (read-after-write) committed but in store queue not yet written to L2D")},
  {"RAW_hit", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("Loads delayed by a previous store (read-after-write hazards)")},
  //0x2f
  {"Cycles_user_non_MLA", NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {"Cycles_user_MLA", NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {"Cycles_user", NULL, REGNO_ANY, NULL, PRELOAD (1, 5), 1, ABST_NONE, STXT ("Cycles hardware thread is active in specified mode(s)")},
  //0x37
  {"DTLB_HWTW_hit_8K", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk successfully loaded translation for 8K page")},
  {"DTLB_HWTW_hit_64K", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk successfully loaded translation for 64K page")},
  {"DTLB_HWTW_hit_4M", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk successfully loaded translation for 4M page")},
  {"DTLB_HWTW_hit_256M", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk successfully loaded translation for 256M page")},
  {"DTLB_HWTW_hit_16G", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk successfully loaded translation for 16G page")},
  {"DTLB_HWTW_hit_1T", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk successfully loaded translation for 1T page")},
  {"DTLB_HWTW_miss_trap", NULL, REGNO_ANY, NULL, PRELOAD (800, 5), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk unsuccessful")},
  {"DTLB_HWTW", NULL, REGNO_ANY, NULL, PRELOAD (40, 5), 0, ABST_NONE, STXT ("DTLB miss and HW tablewalk search done")},
  //0x3f
  {"Commit_0_cyc", /*22825776*/ NULL, 3, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles no uop commits from this hardware thread")},
  {"Commit_0_all_cyc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE, STXT ("Cycles no uop commits from any hardware thread on this core")},
  // Similar situation to Sel_1_cyc etc. No direct equivalent, nearest is Commit_uop, which increments by 0-4 each cycle according to how many uops were committed.

  /* additional (hidden) aliases, for convenience */
  {"cycles0", "Cycles_user", 0, NULL, PRELOADS_8, 1, ABST_NONE},
  {"cycles1", "Cycles_user", 1, NULL, PRELOADS_8, 1, ABST_NONE},
  {"insts0", "Instr_all", 0, NULL, PRELOADS_8, 0, ABST_NONE},
  {"insts1", "Instr_all", 1, NULL, PRELOADS_8, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

static Hwcentry usfuji_V_list[] = {
  {"cycles", "cycle_counts", REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_7, 1, ABST_NONE},
  {"insts", "instruction_counts", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_7, 0, ABST_NONE},
  {"flops", "floating_instructions", REGNO_ANY, STXT ("Floating-point Ops"), PRELOADS_6, 0, ABST_NONE},

  /* explicit definitions of (hidden) entries for proper counters */
  /*  Only counters that can be time converted, or are load-store need to be in this table */
  {"cycle_counts", NULL, REGNO_ANY, NULL, PRELOADS_7, 1, ABST_NONE},
  {"load_store_instructions", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE},

  /* additional (hidden) aliases for convenience */
  {"cycles0", "cycle_counts", 0, NULL, PRELOADS_75, 1, ABST_NONE},
  {"cycles1", "cycle_counts", 1, NULL, PRELOADS_75, 1, ABST_NONE},
  {"insts0", "instruction_counts", 0, NULL, PRELOADS_75, 0, ABST_NONE},
  {"insts1", "instruction_counts", 1, NULL, PRELOADS_75, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

static Hwcentry usfuji_VI_VII_list[] = {
  {"cycles", "cycle_counts", REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_75, 1, ABST_NONE},
  {"insts", "instruction_counts", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_75, 0, ABST_NONE},
  {"dcm", "op_r_iu_req_mi_go", REGNO_ANY, STXT ("L1 D-cache Misses"), PRELOADS_6, 0, ABST_NONE},
  {"dcstall", "op_wait_all", REGNO_ANY, STXT ("L1 D-cache Stall Cycles"), PRELOADS_7, 1, ABST_NONE},
  {"dtlbm", "write_op_uTLB", REGNO_ANY, STXT ("DTLB Misses"), PRELOADS_5, 0, ABST_NONE},
  // l2m: mem_cache_load test shows undercount of 3x, however, we don't care too much about this chip, keeping the alias for now
  {"l2m", "sx_miss_count_dm", REGNO_ANY, STXT ("L2 Cache Misses"), PRELOADS_5, 0, ABST_NONE}, /*YXXX undercounts?*/
  {"l2wm", "dvp_count_dm", REGNO_ANY, STXT ("L2 Cache Writeback Misses"), PRELOADS_5, 0, ABST_NONE},
  {"l2ref", "sx_read_count_dm", REGNO_ANY, STXT ("L2 Cache Refs"), PRELOADS_6, 0, ABST_NONE},
  {"l2stall", "sx_miss_wait_dm", REGNO_ANY, STXT ("L2 Cache Stall Cycles"), PRELOADS_7, 1, ABST_NONE},
  {"icm", "if_r_iu_req_mi_go", REGNO_ANY, STXT ("L1 I-cache Misses"), PRELOADS_6, 0, ABST_NONE},
  {"icstall", "if_wait_all", REGNO_ANY, STXT ("L1 I-cache Stall Cycles"), PRELOADS_7, 1, ABST_NONE},
  {"itlbm", "write_if_uTLB", REGNO_ANY, STXT ("ITLB Misses"), PRELOADS_5, 0, ABST_NONE},
  {"flops", "floating_instructions", REGNO_ANY, STXT ("Floating-point Ops"), PRELOADS_7, 0, ABST_NONE},

  /* explicit definitions of (hidden) entries for proper counters */
  /*  Only counters that can be time converted, or are load-store need to be in this table */
  {"cycle_counts", NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {"op_stv_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"load_store_instructions", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE},
  {"active_cycle_count", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_sxmiss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"branch_comp_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"write_op_uTLB", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE},
  {"sx_miss_wait_pf", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"sx_miss_wait_dm", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_nc_pend", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_sxmiss_ex", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"eu_comp_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"sx_miss_count_dm", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE},
  {"fl_comp_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_r_iu_req_mi_go", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE},
  {"sx_miss_count_dm_if", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE},
  {"op_stv_wait_ex", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"swpf_lbs_hit", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE},
  {"sx_read_count_dm", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE},
  {"trap_DMMU_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE},
  {"op_wait_all", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"sx_miss_count_dm_opex", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE},
  {"if_wait_all", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"dvp_count_dm", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE},
  {"sx_miss_count_dm_opsh", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 0, ABST_NONE},

  /* additional (hidden) aliases for convenience */
  {"cycles0", "cycle_counts", 0, NULL, PRELOADS_8, 1, ABST_NONE},
  {"cycles1", "cycle_counts", 1, NULL, PRELOADS_8, 1, ABST_NONE},
  {"insts0", "instruction_counts", 0, NULL, PRELOADS_8, 0, ABST_NONE},
  {"insts1", "instruction_counts", 1, NULL, PRELOADS_8, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};


static Hwcentry usfuji_X_list[] = {
  {"cycles", "cycle_counts", REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_75, 1, ABST_NONE},
  {"insts", "instruction_counts", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_75, 0, ABST_NONE},
  {"dcm", "L1D_miss", REGNO_ANY, STXT ("L1 D-cache Misses"), PRELOADS_65, 0, ABST_NONE},
  {"dcstall", "L1D_wait_all", REGNO_ANY, STXT ("L1 D-cache Stall Cycles"), PRELOADS_7, 1, ABST_NONE},

  /* explicit definitions of (hidden) entries for proper counters */
  /*  Only counters that can be time converted, or are load-store need to be in this table */
  {"cycle_counts", NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {"w_op_stv_wait_nc_pend", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"eu_comp_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2_miss_wait_pf_bank0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_pfp_busy_ex", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2_miss_wait_dm_bank0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_branch_comp_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_op_stv_wait_sxmiss_ex", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_nc_pend", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2_miss_wait_pf_bank2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_eu_comp_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_op_stv_wait_sxmiss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_sxmiss_ex", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"branch_comp_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2_miss_wait_dm_bank2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"d_move_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_op_stv_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_fl_comp_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_pfp_busy", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"fl_comp_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2_miss_wait_pf_bank1", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_ex", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2_miss_wait_dm_bank1", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_op_stv_wait_ex", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_sxmiss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_swpf", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L1D_wait_all", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2_miss_wait_pf_bank3", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"cse_priority_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_pfp_busy_swpf", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L1I_wait_all", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2_miss_wait_dm_bank3", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {"single_mode_cycle_counts", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"suspend_cycle", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"sleep_cycle", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  /* additional (hidden) aliases for convenience */
  {"cycles0", "cycle_counts", 0, NULL, PRELOADS_8, 1, ABST_NONE},
  {"cycles1", "cycle_counts", 1, NULL, PRELOADS_8, 1, ABST_NONE},
  {"insts0", "instruction_counts", 0, NULL, PRELOADS_8, 0, ABST_NONE},
  {"insts1", "instruction_counts", 1, NULL, PRELOADS_8, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

static Hwcentry usfuji_XII_list[] = {
  {"cycles", "cycle_counts", REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_75, 1, ABST_NONE},
  {"insts", "instruction_counts", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_75, 0, ABST_NONE},
  {"dcm", "L1D_miss", REGNO_ANY, STXT ("L1 D-cache Misses"), PRELOADS_65, 0, ABST_NONE},
  {"dcstall", "L1D_wait_all", REGNO_ANY, STXT ("L1 D-cache Stall Cycles"), PRELOADS_7, 1, ABST_NONE},

  /* explicit definitions of (hidden) entries for proper counters */
  /*  Only counters that can be time converted, or are load-store need to be in this table */
  {"cycle_counts", NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {"L1D_wait_all", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L1I_wait_all", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2_miss_wait_dm_bank0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2_miss_wait_dm_bank1", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2_miss_wait_dm_bank2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2_miss_wait_dm_bank3", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2_miss_wait_pf_bank0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2_miss_wait_pf_bank1", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2_miss_wait_pf_bank2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"L2_miss_wait_pf_bank3", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"LL_miss_wait_dm_bank0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"LL_miss_wait_dm_bank1", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"LL_miss_wait_dm_bank2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"LL_miss_wait_dm_bank3", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"LL_miss_wait_pf_bank0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"LL_miss_wait_pf_bank1", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"LL_miss_wait_pf_bank2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"LL_miss_wait_pf_bank3", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"branch_comp_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"cse_priority_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"d_move_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"eu_comp_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"fl_comp_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"l2_sy_miss_wait_dm_part1", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"l2_sy_miss_wait_dm_part2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"msgr_reqp_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"msgr_rtnp_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"msgs_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_ex", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_l1d_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_l1d_miss_ex", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_l2_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_l2_miss_ex", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_ll_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_ll_miss_ex", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_nc_pend", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_pfp_busy", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_pfp_busy_ex", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_pfp_busy_swpf", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_swpf", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_sxmiss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"op_stv_wait_sxmiss_ex", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_branch_comp_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_eu_comp_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_fl_comp_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_op_stv_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_op_stv_wait_ex", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_op_stv_wait_l1d_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_op_stv_wait_l1d_miss_ex", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_op_stv_wait_l2_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_op_stv_wait_l2_miss_ex", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_op_stv_wait_ll_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_op_stv_wait_ll_miss_ex", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_op_stv_wait_nc_pend", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_op_stv_wait_pfp_busy", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_op_stv_wait_pfp_busy_ex", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_op_stv_wait_pfp_busy_swpf", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_op_stv_wait_sxmiss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"w_op_stv_wait_sxmiss_ex", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {"single_mode_cycle_counts", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"suspend_cycle", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"sleep_cycle", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  /* additional (hidden) aliases for convenience */
  {"cycles0", "cycle_counts", 0, NULL, PRELOADS_8, 1, ABST_NONE},
  {"cycles1", "cycle_counts", 1, NULL, PRELOADS_8, 1, ABST_NONE},
  {"insts0", "instruction_counts", 0, NULL, PRELOADS_8, 0, ABST_NONE},
  {"insts1", "instruction_counts", 1, NULL, PRELOADS_8, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

/* Kernel profiling pseudo-chip, OBSOLETE (To support 12.3 and earlier, TBR) */
static Hwcentry kproflist[] = {
  {"kcycles", "kcycles", 0, STXT ("KCPU Cycles"), PRELOADS_5, 1, ABST_NONE},
  {"kucycles", "kucycles", 0, STXT ("KUCPU Cycles"), PRELOADS_5, 1, ABST_NONE},
  {"kthr", "kthr", 0, STXT ("KTHR Cycles"), PRELOADS_5, 1, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

static Hwcentry pentiumIIlist[] = {
  /*  note -- missing entries for dtlbm, ecm */
  {"cycles", "cpu_clk_unhalted", REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_7, 1, ABST_NONE},
  {"insts", "inst_retired", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_7, 0, ABST_NONE},
  {"icm", "ifu_ifetch_miss", REGNO_ANY, STXT ("I$ Misses"), PRELOADS_5, 0, ABST_NONE},
  {"dcrm", "dcu_m_lines_in", REGNO_ANY, STXT ("D$ Read Misses"), PRELOADS_5, 0, ABST_NONE},
  {"dcwm", "dcu_m_lines_out", REGNO_ANY, STXT ("D$ Write Misses"), PRELOADS_5, 0, ABST_NONE},
  {"flops", "flops", REGNO_ANY, STXT ("Floating-point Ops"), PRELOADS_7, 0, ABST_NONE},
  {"itlbm", "itlb_miss", REGNO_ANY, STXT ("ITLB Misses"), PRELOADS_5, 0, ABST_NONE},
  {"ecim", "l2_ifetch", REGNO_ANY, STXT ("E$ Instr. Misses"), PRELOADS_5, 0, ABST_NONE},

  /* explicit definitions of (hidden) entries for proper counters */
  /*  Only counters that can be time converted, or are load-store need to be in this table */
  {"cpu_clk_unhalted", NULL, REGNO_ANY, NULL, PRELOADS_7, 1, ABST_NONE},

  /* additional (hidden) aliases for convenience */
  {"cycles0", "cpu_clk_unhalted", 0, NULL, PRELOADS_75, 1, ABST_NONE},
  {"cycles1", "cpu_clk_unhalted", 1, NULL, PRELOADS_75, 1, ABST_NONE},
  {"insts0", "inst_retired", 0, NULL, PRELOADS_75, 0, ABST_NONE},
  {"insts1", "inst_retired", 1, NULL, PRELOADS_75, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

static Hwcentry pentiumIIIlist[] = {
  /*  note -- many missing entries; no reference machine to try */
  {"cycles", "cpu_clk_unhalted", REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_7, 1, ABST_NONE},
  {"insts", "inst_retired", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_7, 0, ABST_NONE},

  /* explicit definitions of (hidden) entries for proper counters */
  /*  Only counters that can be time converted, or are load-store need to be in this table */
  {"cpu_clk_unhalted", NULL, REGNO_ANY, NULL, PRELOADS_7, 1, ABST_NONE},

  /* additional (hidden) aliases for convenience */
  {"cycles0", "cpu_clk_unhalted", 0, NULL, PRELOADS_75, 1, ABST_NONE},
  {"cycles1", "cpu_clk_unhalted", 1, NULL, PRELOADS_75, 1, ABST_NONE},
  {"insts0", "inst_retired", 0, NULL, PRELOADS_75, 0, ABST_NONE},
  {"insts1", "inst_retired", 1, NULL, PRELOADS_75, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

static Hwcentry pentium4[] = {
  {"cycles", "TC_deliver_mode~threshold=0xf~complement=1~compare=1", REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_7, 1, ABST_NONE},
  {"insts", "instr_retired~emask=0x3", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_7, 0, ABST_NONE},
  {"l1m", "BSQ_cache_reference~emask=0x0507", REGNO_ANY, STXT ("L1 Cache Misses"), PRELOADS_7, 0, ABST_NONE},
  {"l2h", "BSQ_cache_reference~emask=0x0007", REGNO_ANY, STXT ("L2 Cache Hits"), PRELOADS_7, 0, ABST_NONE},
  {"l2m", "BSQ_cache_reference~emask=0x0500", REGNO_ANY, STXT ("L2 Cache Misses"), PRELOADS_6, 0, ABST_NONE},

  /* explicit definitions of (hidden) entries for proper counters */
  /*  Only counters that can be time converted, or are load-store need to be in this table */
  {"TC_deliver_mode", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"machine_clear", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  /* additional (hidden) aliases, for convenience */
  {"cycles0", "TC_deliver_mode~threshold=0xf~complement=1~compare=1", 5, NULL, PRELOADS_75, 1, ABST_NONE},
  {"cycles1", "TC_deliver_mode~threshold=0xf~complement=1~compare=1", 6, NULL, PRELOADS_75, 1, ABST_NONE},
  {"insts0", "instr_retired~emask=0x3", 15, NULL, PRELOADS_75, 0, ABST_NONE},
  {"insts1", "instr_retired~emask=0x3", 16, NULL, PRELOADS_75, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

static Hwcentry intelCore2list[] = {
  // For post-processing, both Linux and Solaris definitions need to be "live".
  // However, for data collection, OS-specific definitions may need to be hidden.
  // Use REGNO_INVALID for definitions that should be hidden for data collection.
#define LINUX_ONLY   REGNO_ANY
#define SOLARIS_ONLY REGNO_INVALID /* hidden for Linux data collection */

  {"cycles", "cpu_clk_unhalted.core", /*6759307*/ SOLARIS_ONLY, STXT ("CPU Cycles"), PRELOADS_75, 1, ABST_NONE},
  {"cycles", "cpu_clk_unhalted.thread", /*6759307*/ SOLARIS_ONLY, STXT ("CPU Cycles"), PRELOADS_75, 1, ABST_NONE},
  /* Linux Note: 7046312 Many HWC tests fail on system Core2 system with perf_events if above alias used */
  {"cycles", "cpu_clk_unhalted", LINUX_ONLY, STXT ("CPU Cycles"), PRELOADS_75, 1, ABST_NONE},

  {"insts", "instr_retired.any", SOLARIS_ONLY, STXT ("Instructions Executed"), PRELOADS_75, 0, ABST_NONE},
  /* Linux Note: 7046312 Many HWC tests fail on system Core2 system with perf_events if above alias used */
  {"insts", "inst_retired", LINUX_ONLY, STXT ("Instructions Executed"), PRELOADS_75, 0, ABST_NONE},

  // The following counters were identified in "Cycle Accounting Analysis on Intel Core2 Processors" by David Levinthal
  {"uops_stalled", "rs_uops_dispatched~cmask=1~inv=1", REGNO_ANY, STXT ("uOps Stalled"), PRELOADS_7, 1, ABST_NONE},
  {"l2m", "mem_load_retired~umask=0x08", REGNO_ANY, STXT ("L2 Line Misses"), PRELOADS_5, 0, ABST_NONE},
  {"dtlbm", "mem_load_retired~umask=0x10", REGNO_ANY, STXT ("L1 DTLB Misses"), PRELOADS_5, 0, ABST_NONE},
  {"l1m", "mem_load_retired~umask=0x02", REGNO_ANY, STXT ("L1 Line Misses"), PRELOADS_6, 0, ABST_NONE},
  // {"stalls_resources","resource_stalls~umask=0x1f",		REGNO_ANY, STXT("Resource Stalls"),		PRELOADS_6, 1, ABST_NONE},
  {"rs_full", "resource_stalls~umask=0x02", REGNO_ANY, STXT ("Reservation Station Full"), PRELOADS_6, 1, ABST_NONE},
  {"br_miss_flush", "resource_stalls~umask=0x10", REGNO_ANY, STXT ("Mispredicted Branch Flushes"), PRELOADS_6, 1, ABST_NONE},
  {"ld_st_full", "resource_stalls~umask=0x04", REGNO_ANY, STXT ("Load/Store Buffers Full"), PRELOADS_6, 1, ABST_NONE},
  {"rob_full", "resource_stalls~umask=0x01", REGNO_ANY, STXT ("Reorder Buffer Full"), PRELOADS_6, 1, ABST_NONE},
  {"slow_decode", "ild_stall", REGNO_ANY, STXT ("Slow Instruction Decode"), PRELOADS_6, 1, ABST_NONE},
  {"br_miss", "br_cnd_missp_exec", REGNO_ANY, STXT ("Mispredicted Branches"), PRELOADS_5, 0, ABST_NONE},
  {"ret_miss", "br_call_missp_exec", REGNO_ANY, STXT ("Mispredicted Return Calls"), PRELOADS_5, 0, ABST_NONE},
  {"div_busy", "idle_during_div", REGNO_ANY, STXT ("Divider Unit Busy"), PRELOADS_5, 1, ABST_NONE},
  {"fp_assists", "fp_assist", REGNO_ANY, STXT ("FP Microcode Assists"), PRELOADS_5, 0, ABST_NONE},
  {"bus_busy", "bus_drdy_clocks~umask=0x60", REGNO_ANY, STXT ("Busy Data Bus"), PRELOADS_5, 1, ABST_NONE},

  /* explicit definitions of (hidden) entries for proper counters */
  /*  Only counters that can be time converted, or are load-store need to be in this table */
  {/*30a*/"cpu_clk_unhalted.core", /*6759307*/ NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {/*30a*/"cpu_clk_unhalted.thread", /*6759307*/ NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {/*03*/"store_block", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*03*/"store_block.drain_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*03*/"store_block.order", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*03*/"store_block.snoop", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*09*/"memory_disambiguation.reset", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0c*/"page_walks.cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*14*/"cycles_div_busy", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*18*/"idle_during_div", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*19*/"delayed_bypass.load", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*21*/"l2_ads", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*23*/"l2_dbus_busy_rd", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*32*/"l2_no_req", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*3c*/"cpu_clk_unhalted", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*3c*/"cpu_clk_unhalted.core_p", NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {/*3c*/"cpu_clk_unhalted.bus", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*3c*/"cpu_clk_unhalted.no_other", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*42*/"l1d_cache_lock.duration", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*62*/"bus_drdy_clocks", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*63*/"bus_lock_clocks", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*64*/"bus_data_rcv", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*7a*/"bus_hit_drv", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*7b*/"bus_hitm_drv", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*7d*/"busq_empty", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*7e*/"snoop_stall_drv", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*7f*/"bus_io_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*83*/"inst_queue", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*83*/"inst_queue.full", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*86*/"cycles_l1i_mem_stalled", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*87*/"ild_stall", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1*/"rs_uops_dispatched", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1*/"rs_uops_dispatched_port", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1*/"rs_uops_dispatched_port.0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1*/"rs_uops_dispatched_port.1", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1*/"rs_uops_dispatched_port.2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1*/"rs_uops_dispatched_port.3", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1*/"rs_uops_dispatched_port.4", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1*/"rs_uops_dispatched_port.5", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*6c*/"cycles_int", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*6c*/"cycles_int.masked", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*6c*/"cycles_int.pending_and_masked", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*d2*/"rat_stalls", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*d2*/"rat_stalls.rob_read_port", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*d2*/"rat_stalls.partial_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*d2*/"rat_stalls.flags", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*d2*/"rat_stalls.fpsw", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*d2*/"rat_stalls.any", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*d2*/"rat_stalls.other_serialization_stalls", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*d4*/"seg_rename_stalls", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*d4*/"seg_rename_stalls.es", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*d4*/"seg_rename_stalls.ds", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*d4*/"seg_rename_stalls.fs", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*d4*/"seg_rename_stalls.gs", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*d4*/"seg_rename_stalls.any", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*dc*/"resource_stalls", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*dc*/"resource_stalls.rob_full", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*dc*/"resource_stalls.rs_full", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*dc*/"resource_stalls.ld_st", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*dc*/"resource_stalls.fpcw", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*dc*/"resource_stalls.br_miss_clear", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*dc*/"resource_stalls.any", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  /* "Architectural" events: */
  {/*3c*/"unhalted-core-cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  /* additional (hidden) aliases for convenience */
  {"cycles0", "cpu_clk_unhalted", 0, NULL, PRELOADS_8, 1, ABST_NONE},
  {"cycles1", "cpu_clk_unhalted", 1, NULL, PRELOADS_8, 1, ABST_NONE},
  {"insts0", "inst_retired", 0, NULL, PRELOADS_8, 0, ABST_NONE},
  {"insts1", "inst_retired", 1, NULL, PRELOADS_8, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};


static Hwcentry intelNehalemList[] = {
  /* 6832635: on Linux, we're not seeing consistent overflows on FFCs */
  /* 15634344==6940930: HWC overflow profiling can cause system hang on Solaris/core-i7 systems */
  /* 17578620: counter overflow for fixed-function counters hangs systems */
  /* same issues for intelSandyBridgeList and intelHaswellList */
  PERF_EVENTS_SW_EVENT_ALIASES
  USE_INTEL_REF_CYCLES (133)
  {"cycles", "cpu_clk_unhalted.thread_p", REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_75, 1, ABST_NONE},
  {"insts", "inst_retired.any_p", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_75, 0, ABST_NONE},
  // cpu_clk_unhalted.ref: at the ref requency of the cpu. Should not be affected by Speedstep or Turbo.
  // cpu_clk_unhalted.thread_p: with HT & 2 threads, 2x cycles.  Affected by Speedstep and Turbo.

  // PEBs (Sampling)
  {"l2m_latency", "mem_inst_retired.latency_above_threshold", REGNO_ANY, STXT ("L2 Cache Miss Est. Latency"), PRELOADS_4, 33, ABST_EXACT_PEBS_PLUS1},

  // See file hwctable.README.corei7
  {"dch", "mem_load_retired.l1d_hit", REGNO_ANY, STXT ("L1 D-cache Hits"), PRELOADS_7, 0, ABST_NONE},
  {"dcm", "0xCB~umask=0x1e", REGNO_ANY, STXT ("L1 D-Cache Misses"), PRELOADS_65, 0, ABST_NONE}, /*mem_load_retired*/
  {"lfbdh", "mem_load_retired.hit_lfb", REGNO_ANY, STXT ("LFB D-cache Hits"), PRELOADS_65, 0, ABST_NONE},
  {"l2h", "mem_load_retired.l2_hit", REGNO_ANY, STXT ("L2 Cache Hits"), PRELOADS_65, 0, ABST_NONE},
  {"l2m", "0xCB~umask=0x1c", REGNO_ANY, STXT ("L2 Cache Misses"), PRELOADS_6, 0, ABST_NONE}, /*mem_load_retired*/
  {"l3h", "mem_load_retired.llc_unshared_hit", REGNO_ANY, STXT ("L3 Cache Hit w/o Snoop"), PRELOADS_6, 0, ABST_NONE},
  {"l3h_stall", "mem_load_retired.llc_unshared_hit", REGNO_ANY, STXT ("L3 Cache Hit w/o Snoop x 35: Est. Stalls"), PRELOADS_6, 35, ABST_NONE},
  {"l3hsnoop", "mem_load_retired.other_core_l2_hit_hitm", REGNO_ANY, STXT ("L3 Cache Hit w/Snoop"), PRELOADS_6, 0, ABST_NONE},
  {"l3hsnoop_stall", "mem_load_retired.other_core_l2_hit_hitm", REGNO_ANY, STXT ("L3 Cache Hit w/Snoop x 74: Est. Stalls"), PRELOADS_6, 74, ABST_NONE},
  {"l3m", "mem_load_retired.llc_miss", REGNO_ANY, STXT ("L3 Cache Misses"), PRELOADS_5, 0, ABST_NONE},
  {"l3m_stall", "mem_load_retired.llc_miss", REGNO_ANY, STXT ("L3 Cache Misses x 180: Estimated Stalls"), PRELOADS_5, 180, ABST_NONE},
  {"dtlbm", "dtlb_load_misses.walk_completed", REGNO_ANY, STXT ("DTLB Misses"), PRELOADS_6, 0, ABST_NONE},
  {"dtlbm_stall", "dtlb_load_misses.walk_completed", REGNO_ANY, STXT ("DTLB Misses x 30: Estimated Stalls"), PRELOADS_6, 30, ABST_NONE},
  {"addr_alias_stall", "partial_address_alias", REGNO_ANY, STXT ("Partial Address Aliases x 3: Est. Stalls"), PRELOADS_6, 3, ABST_NONE},
  {"uope_stall", "uops_executed.port234~cmask=1~inv=1", REGNO_ANY, STXT ("UOP Execute Stalls per Core"), PRELOADS_7, 1, ABST_NONE},
  {"uopr_stall", "uops_retired.any~cmask=1~inv=1", REGNO_ANY, STXT ("UOP Retired Stalls"), PRELOADS_7, 1, ABST_NONE},
  {"itlbm", "itlb_miss_retired", REGNO_ANY, STXT ("ITLB Misses"), PRELOADS_6, 0, ABST_NONE},
  {"l1i_stall", "l1i.cycles_stalled", REGNO_ANY, STXT ("L1 I-cache Stalls"), PRELOADS_6, 1, ABST_NONE},
  {"br_rets", "br_inst_retired.all_branches", REGNO_ANY, STXT ("Branch Instruction Retires"), PRELOADS_7, 0, ABST_NONE},
  {"br_misp", "br_misp_exec.any", REGNO_ANY, STXT ("Branch Mispredicts"), PRELOADS_6, 0, ABST_NONE},
  {"mach_clear", "machine_clears.cycles", REGNO_ANY, STXT ("Machine Clear Asserted"), PRELOADS_6, 1, ABST_NONE},
  {"fp_mmx", "fp_mmx_trans.any", REGNO_ANY, STXT ("FP-MMX Transistions"), PRELOADS_6, 0, ABST_NONE},
  {"div_busy", "arith.cycles_div_busy", REGNO_ANY, STXT ("Divider Busy Cycles"), PRELOADS_6, 1, ABST_NONE},

  /* explicit definitions of (hidden) entries for proper counters */
  /*  Only counters that can be time converted, or are load-store need to be in this table */
  {/*30a*/"cpu_clk_unhalted.core", /*6759307*/ NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {/*30a*/"cpu_clk_unhalted.thread", /*6759307*/ NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {/*04*/"sb_drain.cycles", /*future*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*08.04*/"dtlb_load_misses.walk_cycles", /*westmere*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  //{/*0e*/"uops_issued.stalled_cycles",/*future, multibit*/		NULL, REGNO_ANY, NULL, PRELOAD_DEF,     1, ABST_NONE},
  {/*09*/"memory_disambiguation.reset", /*future*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*09*/"memory_disambiguation.watch_cycles", /*future*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0b*/"mem_inst_retired.latency_above_threshold", /*PEBS*/ NULL, REGNO_ANY, NULL, PRELOADS_4, 33, ABST_EXACT_PEBS_PLUS1}, //non-standard overflow
  {/*14*/"arith.cycles_div_busy", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*17*/"inst_queue_write_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*1d*/"hw_int.cycles_masked", /*future*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*1d*/"hw_int.cycles_pending_and_masked", /*future*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*3c*/"cpu_clk_unhalted.thread_p", NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {/*48*/"l1d_pend_miss.load_buffers_full", /*future*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*49.04*/"dtlb_misses.walk_cycles", /*westmere*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*4e*/"sfence_cycles", /*future*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*4f.10*/"ept.walk_cycles", /*westmere*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60*/"offcore_requests_outstanding.demand.read_data", /*future*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60*/"offcore_requests_outstanding.demand.read_code", /*future*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60*/"offcore_requests_outstanding.demand.rfo", /*future*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60*/"offcore_requests_outstanding.any.read", /*future*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*63*/"cache_lock_cycles.l1d", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*63*/"cache_lock_cycles.l1d_l2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*80*/"l1i.cycles_stalled", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*85*/"itlb_misses.walk_cycles", /*future*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*85*/"itlb_misses.pmh_busy_cycles", /*future*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*87*/"ild_stall.lcp", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*87*/"ild_stall.mru", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*87*/"ild_stall.iq_full", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*87*/"ild_stall.regen", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*87*/"ild_stall.any", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2*/"resource_stalls.any", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2*/"resource_stalls.load", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2*/"resource_stalls.rs_full", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2*/"resource_stalls.store", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2*/"resource_stalls.rob_full", /*future*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2*/"resource_stalls.fpcw", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2*/"resource_stalls.mxcsr", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2*/"resource_stalls.other", /*future*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b0*/"offcore_requests_sq_full", /*future*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b3*/"snoopq_requests_outstanding.data", /*future*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b3*/"snoopq_requests_outstanding.invalidate", /*future*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b3*/"snoopq_requests_outstanding.code", /*future*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  //{/*c2*/"uops_retired.stalled_cycles",/*future, multibit*/		NULL, REGNO_ANY, NULL, PRELOAD_DEF,     1, ABST_NONE},
  {/*c3*/"machine_clears.cycles", /*future*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*d2*/"rat_stalls.flags", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*d2*/"rat_stalls.registers", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*d2*/"rat_stalls.rob_read_port", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*d2*/"rat_stalls.scoreboard", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*d2*/"rat_stalls.any", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*d4*/"seg_rename_stalls", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*f6*/"sq_full_stall_cycles", /*future*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  /* "Architectural" events: */
  {/*3c*/"unhalted-core-cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  PERF_EVENTS_SW_EVENT_DEFS

  /* additional (hidden) aliases for convenience */
#if 0
  USE_INTEL_REF_CYCLES (133),
#endif
  {"insts0", "inst_retired.any_p", 0, NULL, PRELOADS_8, 0, ABST_NONE},
  {"insts1", "inst_retired.any_p", 1, NULL, PRELOADS_8, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};


static Hwcentry intelSandyBridgeList[] = {
  /* see comments for "cycles" and "insts" for intelNehalemList */
  PERF_EVENTS_SW_EVENT_ALIASES
  USE_INTEL_REF_CYCLES (100)
  {"cycles", "cpu_clk_unhalted.thread_p", REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_75, 1, ABST_NONE},
  {"insts", "inst_retired.any_p", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_75, 0, ABST_NONE},

  // PEBS (sampling)
  {"l2m_latency", "mem_trans_retired.load_latency", REGNO_ANY, STXT ("L2 Cache Miss Est. Latency"), PRELOADS_4, 65, ABST_EXACT_PEBS_PLUS1},

  // See file hwctable.README.sandybridge
  {"dch", "mem_load_uops_retired.l1_hit", REGNO_ANY, STXT ("L1 D-cache Hits"), PRELOADS_7, 0, ABST_NONE},
  {"dcm", "mem_load_uops_retired.l1_miss", REGNO_ANY, STXT ("L1 D-cache Misses"), PRELOADS_65, 0, ABST_NONE}, /*mem_load_uops_retired*/
  {"l2h", "mem_load_uops_retired.l2_hit", REGNO_ANY, STXT ("L2 Cache Hits"), PRELOADS_65, 0, ABST_NONE},
  {"l2m", "mem_load_uops_retired.l2_miss", REGNO_ANY, STXT ("L2 Cache Misses"), PRELOADS_6, 0, ABST_NONE}, /*mem_load_uops_retired*/
  // Intel errata:  BT241 and BT243 says the mem_load_uops_retired.llc* counters may not be reliable on some CPU variants
  {"l3h", "mem_load_uops_retired.llc_hit", REGNO_ANY, STXT ("L3 Cache Hit w/o Snoop"), PRELOADS_6, 0, ABST_NONE}, // may undercount
  {"l3m", "longest_lat_cache.miss", REGNO_ANY, STXT ("L3 Cache Misses"), PRELOADS_5, 0, ABST_NONE},

  /* dtlbm has not been confirmed via Intel white paper */
  {"dtlbm", "dtlb_load_misses.walk_completed", REGNO_ANY, STXT ("DTLB Misses"), PRELOADS_6, 0, ABST_NONE},
  {"dtlbm_stall", "dtlb_load_misses.walk_completed", REGNO_ANY, STXT ("DTLB Misses x 30: Estimated Stalls"), PRELOADS_6, 30, ABST_NONE},
  {"dtlbm", "dtlb_load_misses.demand_ld_walk_completed", REGNO_ANY, STXT ("DTLB Misses"), PRELOADS_6, 0, ABST_NONE},
  {"dtlbm_stall", "dtlb_load_misses.demand_ld_walk_completed", REGNO_ANY, STXT ("DTLB Misses x 30: Estimated Stalls"), PRELOADS_6, 30, ABST_NONE},

  /* explicit definitions of (hidden) entries for proper counters */
  /*  Only counters that can be time converted, or are load-store need to be in this table */
  {/* 30a */"cpu_clk_unhalted.thread", /*15634344==6940930*/ NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  //{/* 30a */"cpu_clk_unhalted.core",  /*6759307*/			NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {/*08.04*/"dtlb_load_misses.walk_duration", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*08.84*/"dtlb_load_misses.demand_ld_walk_duration", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0d.03*/"int_misc.recovery_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0d.40*/"int_misc.rat_stall_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0e.01*/"uops_issued.stall_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0e.01*/"uops_issued.core_stall_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*14.01*/"arith.fpu_div_active", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*3c.00*/"cpu_clk_unhalted.thread_p", NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {/*48.01*/"l1d_pend_miss.pending_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*49.04*/"dtlb_store_misses.walk_duration", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*59.20*/"partial_rat_stalls.flags_merge_uop", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*59.20*/"partial_rat_stalls.flags_merge_uop_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*59.40*/"partial_rat_stalls.slow_lea_window", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  //{/*59.80*/"partial_rat_stalls.mul_single_uop",			NULL, REGNO_ANY, NULL, PRELOAD_DEF,     1, ABST_NONE},
  {/*5b.0c*/"resource_stalls2.all_fl_empty", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*5b.0f*/"resource_stalls2.all_prf_control", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*5b.40*/"resource_stalls2.bob_full", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*5b.4f*/"resource_stalls2.ooo_rsrc", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*5c.01*/"cpl_cycles.ring0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*5c.02*/"cpl_cycles.ring123", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*5c.xx*/"cpl_cycles.ring0_trans", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*5c.xx*/"cpl_cycles.ring0_transition", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*5e.01*/"rs_events.empty_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60.01*/"offcore_requests_outstanding.cycles_with_demand_data_rd", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60.01*/"offcore_requests_outstanding.demand_data_rd_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60.04*/"offcore_requests_outstanding.cycles_with_demand_rfo", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60.04*/"offcore_requests_outstanding.demand_rfo_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60.08*/"offcore_requests_outstanding.cycles_with_data_rd", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60.08*/"offcore_requests_outstanding.all_data_rd_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60.02*/"offcore_requests_outstanding.demand_code_rd_cycles", /*?*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*63.01*/"lock_cycles.split_lock_uc_lock_duration", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*63.02*/"lock_cycles.cache_lock_duration", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.00*/"idq.empty", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.04*/"idq.mite_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.08*/"idq.dsb_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.10*/"idq.ms_dsb_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.20*/"idq.ms_mite_uops_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.20*/"idq.ms_mite_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.30*/"idq.ms_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.18*/"idq.all_dsb_cycles_any_uops", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.18*/"idq.all_dsb_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.18*/"idq.all_dsb_cycles_4_uops", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.24*/"idq.all_mite_cycles_any_uops", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.24*/"idq.all_mite_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.24*/"idq.all_mite_cycles_4_uops", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.3c*/"idq.mite_all_cycles", /* Linux, but not in docs? */ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*80.04*/"icache.ifetch_stall", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*85.04*/"itlb_misses.walk_duration", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*87.01*/"ild_stall.lcp", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*87.04*/"ild_stall.iq_full", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*9c.xx*/"idq_uops_not_delivered.cycles_0_uops_deliv.core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*9c.xx*/"idq_uops_not_delivered.cycles_le_1_uop_deliv.core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*9c.xx*/"idq_uops_not_delivered.cycles_le_2_uop_deliv.core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*9c.xx*/"idq_uops_not_delivered.cycles_le_3_uop_deliv.core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*9c.01*/"idq_uops_not_delivered.cycles_ge_1_uop_deliv.core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*9c.01*/"idq_uops_not_delivered.cycles_fe_was_ok", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.01*/"uops_executed_port.port_0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.02*/"uops_executed_port.port_1", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.04*/"uops_executed_port.port_2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.08*/"uops_executed_port.port_3", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.10*/"uops_executed_port.port_4", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.20*/"uops_executed_port.port_5", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2.01*/"resource_stalls.any", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2.02*/"resource_stalls.lb", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2.04*/"resource_stalls.rs", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2.08*/"resource_stalls.sb", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2.0a*/"resource_stalls.lb_sb", /*sb-ep*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2.0e*/"resource_stalls.mem_rs", /*sb-ep*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2.10*/"resource_stalls.rob", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2.20*/"resource_stalls.fcsw", /*sb*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2.40*/"resource_stalls.mxcsr", /*sb*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2.80*/"resource_stalls.other", /*sb*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2.F0*/"resource_stalls.ooo_rsrc", /*sb-ep*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {/*a3.01*/"cycle_activity.cycles_l2_pending", /*F6M62*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*??.??*/"cycle_activity.stalls_l2_pending", /*F6M62*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a3.02*/"cycle_activity.cycles_ldm_pending", /*F6M62*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*??.??*/"cycle_activity.stalls_ldm_pending", /*F6M62*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a3.04*/"cycle_activity.cycles_no_execute", /*F6M62*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a3.04*/"cycle_activity.cycles_no_dispatch", /*sandybridge*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a3.08*/"cycle_activity.cycles_l1d_pending", /*F6M62*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*??.??*/"cycle_activity.stalls_l1d_pending", /*F6M62*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {/*ab.02*/"dsb2mite_switches.penalty_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b1.??*/"uops_executed.stall_cycles", /*? not in PRM*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b1.01*/"uops_dispatched.stall_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b1.01*/"uops_executed.stall_cycles", /*F6M62*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b1.01*/"uops_executed.cycles_ge_1_uop_exec", /*F6M62,not doc'd*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b1.01*/"uops_executed.cycles_ge_2_uops_exec", /*F6M62,not doc'd*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b1.01*/"uops_executed.cycles_ge_3_uops_exec", /*F6M62,not doc'd*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b1.01*/"uops_executed.cycles_ge_4_uops_exec", /*F6M62,not doc'd*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {/*bf.05*/"l1d_blocks.bank_conflict_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*c2.01*/"uops_retired.stall_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*c2.01*/"uops_retired.total_cycles", /*cmask==0x10*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*c2.01*/"uops_retired.core_stall_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*c2.01*/"uops_retired.active_cycles", /*cmask==0x1*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
#if 0 // need to see documentation on the following before marking them as cycles
  uops_executed.cycles_ge_1_uop_exec[ / {0 | 1 | 2 | 3}], 1000003 (events)
  uops_executed.cycles_ge_2_uops_exec[ /
  {0 | 1 | 2 | 3}
  ], 1000003 (events)
  uops_executed.cycles_ge_3_uops_exec[ /
  {0 | 1 | 2 | 3}
  ], 1000003 (events)
  uops_executed.cycles_ge_4_uops_exec[ /
  {0 | 1 | 2 | 3}
  ], 1000003 (events)
#endif
  {/*cd.01*/"mem_trans_retired.load_latency", /*PEBS*/ NULL, REGNO_ANY, NULL, PRELOADS_4, 65, ABST_EXACT_PEBS_PLUS1}, //non-standard overflow

  /* "Architectural" events: */
  {/*3c*/"unhalted-core-cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  PERF_EVENTS_SW_EVENT_DEFS

  /* additional (hidden) aliases for convenience */
#if 0
  USE_INTEL_REF_CYCLES (100),
#endif
  {"insts0", "inst_retired.any_p", 0, NULL, PRELOADS_8, 0, ABST_NONE},
  {"insts1", "inst_retired.any_p", 1, NULL, PRELOADS_8, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};


static Hwcentry intelHaswellList[] = {
  /* see comments for "cycles" and "insts" for intelNehalemList */
  PERF_EVENTS_SW_EVENT_ALIASES
  USE_INTEL_REF_CYCLES (100)
  {"cycles", "cpu_clk_unhalted.thread_p", REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_75, 1, ABST_NONE},
  {"insts", "inst_retired.any_p", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_75, 0, ABST_NONE},

  // PEBS (sampling)
  {"l2m_latency", "mem_trans_retired.load_latency", REGNO_ANY, STXT ("L2 Cache Miss Est. Latency"), PRELOADS_4, 65, ABST_EXACT_PEBS_PLUS1},

  {"dch", "mem_load_uops_retired.l1_hit", REGNO_ANY, STXT ("L1 D-cache Hits"), PRELOADS_7, 0, ABST_NONE},
  {"dcm", "mem_load_uops_retired.l1_miss", REGNO_ANY, STXT ("L1 D-cache Misses"), PRELOADS_65, 0, ABST_NONE}, //mem_load_uops_retired
  {"dcm", "0xd1~umask=0x08", REGNO_ANY, STXT ("L1 D-cache Misses"), PRELOADS_65, 0, ABST_NONE}, //mem_load_uops_retired
  {"l2h", "mem_load_uops_retired.l2_hit", REGNO_ANY, STXT ("L2 Cache Hits"), PRELOADS_65, 0, ABST_NONE},
  {"l2m", "mem_load_uops_retired.l2_miss", REGNO_ANY, STXT ("L2 Cache Misses"), PRELOADS_6, 0, ABST_NONE}, //mem_load_uops_retired
  {"l2m", "0xd1~umask=0x10", REGNO_ANY, STXT ("L2 Cache Misses"), PRELOADS_6, 0, ABST_NONE}, //mem_load_uops_retired
  {"l3h", "mem_load_uops_retired.l3_hit", REGNO_ANY, STXT ("L3 Cache Hit w/o Snoop"), PRELOADS_6, 0, ABST_NONE},
  {"l3m", "mem_load_uops_retired.l3_miss", REGNO_ANY, STXT ("L3 Cache Misses"), PRELOADS_5, 0, ABST_NONE}, //mem_load_uops_retired
  {"l3m", "0xd1~umask=0x20", REGNO_ANY, STXT ("L3 Cache Misses"), PRELOADS_5, 0, ABST_NONE}, //mem_load_uops_retired

  /* dtlbm has not been confirmed via Intel white paper */
  {"dtlbm", "dtlb_load_misses.walk_completed", REGNO_ANY, STXT ("DTLB Misses"), PRELOADS_6, 0, ABST_NONE},
  {"dtlbm_stall", "dtlb_load_misses.walk_completed", REGNO_ANY, STXT ("DTLB Misses x 30: Estimated Stalls"), PRELOADS_6, 30, ABST_NONE},

  /* explicit definitions of (hidden) entries for proper counters */
  /*  Only counters that can be time converted, or are load-store need to be in this table */
  {/* 30a */"cpu_clk_unhalted.thread", /*15634344==6940930*/ NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  //{/* 30a */"cpu_clk_unhalted.core",  /*6759307*/			NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {/*08.10*/"dtlb_load_misses.walk_duration", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0d.03*/"int_misc.recovery_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0e.01*/"uops_issued.stall_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0e.01*/"uops_issued.core_stall_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*3c.00*/"cpu_clk_unhalted.thread_p", NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {/*48.01*/"l1d_pend_miss.pending_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*49.04*/"dtlb_store_misses.walk_duration", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*5c.01*/"cpl_cycles.ring0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*5c.02*/"cpl_cycles.ring123", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*5c.xx*/"cpl_cycles.ring0_trans", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*5e.01*/"rs_events.empty_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60.01*/"offcore_requests_outstanding.cycles_with_demand_data_rd", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60.02*/"offcore_requests_outstanding.demand_code_rd_cycles", /*?*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60.04*/"offcore_requests_outstanding.demand_rfo_cycles", /*?*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60.08*/"offcore_requests_outstanding.cycles_with_data_rd", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*63.01*/"lock_cycles.split_lock_uc_lock_duration", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*63.02*/"lock_cycles.cache_lock_duration", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.00*/"idq.empty", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.04*/"idq.mite_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.08*/"idq.dsb_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.10*/"idq.ms_dsb_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.20*/"idq.ms_mite_uops_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.20*/"idq.ms_mite_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.30*/"idq.ms_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.18*/"idq.all_dsb_cycles_any_uops", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.18*/"idq.all_dsb_cycles_4_uops", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.24*/"idq.all_mite_cycles_any_uops", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.24*/"idq.all_mite_cycles_4_uops", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*80.04*/"icache.ifetch_stall", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*85.04*/"itlb_misses.walk_duration", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*87.01*/"ild_stall.lcp", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE}, // Intel SDM says these are stalls, not cycles
  {/*87.04*/"ild_stall.iq_full", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*9c.xx*/"idq_uops_not_delivered.cycles_0_uops_deliv.core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*9c.xx*/"idq_uops_not_delivered.cycles_le_1_uop_deliv.core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*9c.xx*/"idq_uops_not_delivered.cycles_le_2_uop_deliv.core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*9c.xx*/"idq_uops_not_delivered.cycles_le_3_uop_deliv.core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  //  {/*9c.01*/"idq_uops_not_delivered.cycles_ge_1_uop_deliv.core",	NULL, REGNO_ANY, NULL, PRELOAD_DEF,     1, ABST_NONE},
  {/*9c.01*/"idq_uops_not_delivered.cycles_fe_was_ok", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {/*a1.01*/"uops_executed_port.port_0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.02*/"uops_executed_port.port_1", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.04*/"uops_executed_port.port_2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.08*/"uops_executed_port.port_3", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.10*/"uops_executed_port.port_4", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.20*/"uops_executed_port.port_5", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.40*/"uops_executed_port.port_6", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.80*/"uops_executed_port.port_7", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.01*/"uops_executed_port.port_0_core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.02*/"uops_executed_port.port_1_core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.04*/"uops_executed_port.port_2_core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.08*/"uops_executed_port.port_3_core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.10*/"uops_executed_port.port_4_core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.20*/"uops_executed_port.port_5_core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.40*/"uops_executed_port.port_6_core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.80*/"uops_executed_port.port_7_core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {/*a2.01*/"resource_stalls.any", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2.04*/"resource_stalls.rs", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2.08*/"resource_stalls.sb", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2.10*/"resource_stalls.rob", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {/*a3.01*/"cycle_activity.cycles_l2_pending_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  //  {/*a3.01*/"cycle_activity.cycles_l2_pending",			NULL, REGNO_ANY, NULL, PRELOAD_DEF,     1, ABST_NONE},
  {/*a3.02*/"cycle_activity.cycles_ldm_pending_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  //  {/*a3.05*/"cycle_activity.stalls_l2_pending",			NULL, REGNO_ANY, NULL, PRELOAD_DEF,     1, ABST_NONE},
  {/*a3.08*/"cycle_activity.cycles_l1d_pending_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  //  {/*a3.??*/"cycle_activity.cycles_no_execute",			NULL, REGNO_ANY, NULL, PRELOAD_DEF,     1, ABST_NONE},
  //  {/*a3.??*/"cycle_activity.stalls_ldm_pending",/*?*/			NULL, REGNO_ANY, NULL, PRELOAD_DEF,     1, ABST_NONE},

  {/*ab.02*/"dsb2mite_switches.penalty_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {/*b1.??*/"uops_executed.stall_cycles", /*? not in PRM*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b1.??*/"uops_executed.cycles_ge_1_uop_exec", /*?*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b1.??*/"uops_executed.cycles_ge_2_uops_exec", /*?*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b1.??*/"uops_executed.cycles_ge_3_uops_exec", /*?*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b1.??*/"uops_executed.cycles_ge_4_uops_exec", /*?*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {/*c2.01*/"uops_retired.stall_cycles", /*cmask==1 + INV*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*c2.01*/"uops_retired.total_cycles", /*cmask==0x1*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*c2.01*/"uops_retired.core_stall_cycles", /*PEBS Any==1*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {/*c3.01*/"machine_clears.cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {/*ca.1e*/"fp_assist.any", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  {/*cd.01*/"mem_trans_retired.load_latency", /*PEBS*/ NULL, REGNO_ANY, NULL, PRELOADS_4, 65, ABST_EXACT_PEBS_PLUS1}, //non-standard overflow

  /* "Architectural" events: */
  {/*3c*/"unhalted-core-cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  PERF_EVENTS_SW_EVENT_DEFS

  /* additional (hidden) aliases for convenience */
#if 0
  USE_INTEL_REF_CYCLES (100),
#endif
  {"insts0", "inst_retired.any_p", 0, NULL, PRELOADS_8, 0, ABST_NONE},
  {"insts1", "inst_retired.any_p", 1, NULL, PRELOADS_8, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};


static Hwcentry intelBroadwellList[] = {
  /* see comments for "cycles" and "insts" for intelNehalemList */
  PERF_EVENTS_SW_EVENT_ALIASES
  USE_INTEL_REF_CYCLES (100)
  {"cycles", "cpu_clk_unhalted.thread_p", REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_75, 1, ABST_NONE},
  {"insts", "inst_retired.any_p", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_75, 0, ABST_NONE},

  // PEBS (sampling)
  {"l2m_latency", "mem_trans_retired.load_latency", REGNO_ANY, STXT ("L2 Cache Miss Est. Latency"), PRELOADS_4, 65, ABST_EXACT_PEBS_PLUS1},
  {/*cd.01*/"mem_trans_retired.load_latency", NULL, REGNO_ANY, NULL, PRELOADS_4, 65, ABST_EXACT_PEBS_PLUS1},

  // aliases (the first set are PEBS, but on Intel the only precise counter we support is l2m_latency)
  {"dch", "mem_load_uops_retired.l1_hit", REGNO_ANY, STXT ("L1 D-cache Hits"), PRELOADS_7, 0, ABST_NONE},
  {"dcm", "mem_load_uops_retired.l1_miss", REGNO_ANY, STXT ("L1 D-cache Misses"), PRELOADS_65, 0, ABST_NONE},
  {"l2h", "mem_load_uops_retired.l2_hit", REGNO_ANY, STXT ("L2 Cache Hits"), PRELOADS_65, 0, ABST_NONE},
  {"l2m", "mem_load_uops_retired.l2_miss", REGNO_ANY, STXT ("L2 Cache Misses"), PRELOADS_6, 0, ABST_NONE},
  {"l3h", "mem_load_uops_retired.l3_hit", REGNO_ANY, STXT ("L3 Cache Hits"), PRELOADS_6, 0, ABST_NONE},
  {"l3m", "mem_load_uops_retired.l3_miss", REGNO_ANY, STXT ("L3 Cache Misses"), PRELOADS_5, 0, ABST_NONE},
  {"dtlbm", "dtlb_load_misses.walk_completed", REGNO_ANY, STXT ("DTLB Misses"), PRELOADS_6, 0, ABST_NONE},

  // counters that can be time converted (add FFCs if we decide to support them)
  // counters that are load-store (did not include any... do we want to?)
  {/*08.10*/"dtlb_load_misses.walk_duration", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0d.03*/"int_misc.recovery_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0e.01*/"uops_issued.stall_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0e.01*/"uops_issued.core_stall_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*14.01*/"arith.fpu_div_active", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*3c.00*/"cpu_clk_unhalted.thread_p_any", NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {/*3c.00*/"cpu_clk_unhalted.thread_p", NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {/*3c.02*/"cpu_clk_thread_unhalted.one_thread_active", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*48.01*/"l1d_pend_miss.pending_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*48.01*/"l1d_pend_miss.pending_cycles_any", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*49.10*/"dtlb_store_misses.walk_duration", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*4f.10*/"ept.walk_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*5c.01*/"cpl_cycles.ring0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*5c.01*/"cpl_cycles.ring0_trans", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*5c.02*/"cpl_cycles.ring123", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*5e.01*/"rs_events.empty_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60.01*/"offcore_requests_outstanding.cycles_with_demand_data_rd", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60.02*/"offcore_requests_outstanding.demand_code_rd_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60.04*/"offcore_requests_outstanding.demand_rfo_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60.08*/"offcore_requests_outstanding.cycles_with_data_rd", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*63.01*/"lock_cycles.split_lock_uc_lock_duration", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*63.02*/"lock_cycles.cache_lock_duration", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.02*/"idq.empty", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.04*/"idq.mite_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.08*/"idq.dsb_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.10*/"idq.ms_dsb_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.18*/"idq.all_dsb_cycles_4_uops", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.18*/"idq.all_dsb_cycles_any_uops", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.24*/"idq.all_mite_cycles_4_uops", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.24*/"idq.all_mite_cycles_any_uops", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.30*/"idq.ms_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*85.10*/"itlb_misses.walk_duration", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*9c.xx*/"idq_uops_not_delivered.cycles_0_uops_deliv.core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*9c.xx*/"idq_uops_not_delivered.cycles_le_1_uop_deliv.core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*9c.xx*/"idq_uops_not_delivered.cycles_le_2_uop_deliv.core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*9c.xx*/"idq_uops_not_delivered.cycles_le_3_uop_deliv.core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*9c.01*/"idq_uops_not_delivered.cycles_fe_was_ok", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.01*/"uops_executed_port.port_0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.02*/"uops_executed_port.port_1", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.04*/"uops_executed_port.port_2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.08*/"uops_executed_port.port_3", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.10*/"uops_executed_port.port_4", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.20*/"uops_executed_port.port_5", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.40*/"uops_executed_port.port_6", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.80*/"uops_executed_port.port_7", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.01*/"uops_executed_port.port_0_core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.02*/"uops_executed_port.port_1_core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.04*/"uops_executed_port.port_2_core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.08*/"uops_executed_port.port_3_core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.10*/"uops_executed_port.port_4_core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.20*/"uops_executed_port.port_5_core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.40*/"uops_executed_port.port_6_core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.80*/"uops_executed_port.port_7_core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2.01*/"resource_stalls.any", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2.04*/"resource_stalls.rs", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2.08*/"resource_stalls.sb", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2.10*/"resource_stalls.rob", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a3.01*/"cycle_activity.cycles_l2_pending", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a3.02*/"cycle_activity.cycles_ldm_pending", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a3.04*/"cycle_activity.cycles_no_execute", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a3.08*/"cycle_activity.cycles_l1d_pending", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a8.01*/"lsd.cycles_active", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a8.01*/"lsd.cycles_4_uops", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*ab.02*/"dsb2mite_switches.penalty_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b1.01*/"uops_executed.stall_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*c2.01*/"uops_retired.stall_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*c2.01*/"uops_retired.total_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*c2.01*/"uops_retired.core_stall_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*c3.01*/"machine_clears.cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*ca.1e*/"fp_assist.any", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  /* "Architectural" events: */
  {/*3c*/"unhalted-core-cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  PERF_EVENTS_SW_EVENT_DEFS

  /* additional (hidden) aliases for convenience */
#if 0
  USE_INTEL_REF_CYCLES (100),
#endif
  {"insts0", "inst_retired.any_p", 0, NULL, PRELOADS_8, 0, ABST_NONE},
  {"insts1", "inst_retired.any_p", 1, NULL, PRELOADS_8, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

static Hwcentry intelSkylakeList[] = {
  /* see comments for "cycles" and "insts" for intelNehalemList */
  PERF_EVENTS_SW_EVENT_ALIASES
  USE_INTEL_REF_CYCLES (25)
  {"cycles", "cpu_clk_unhalted.thread_p", REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_75, 1, ABST_NONE},
  {"insts", "inst_retired.any_p", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_75, 0, ABST_NONE},

  // PEBS (sampling)
  {"l2m_latency", "mem_trans_retired.load_latency", REGNO_ANY, STXT ("L2 Cache Miss Est. Latency"), PRELOADS_4, 65, ABST_EXACT_PEBS_PLUS1},
  {/*cd.01*/"mem_trans_retired.load_latency", NULL, REGNO_ANY, NULL, PRELOADS_4, 65, ABST_EXACT_PEBS_PLUS1},

  // aliases (the first set are PEBS, but on Intel the only precise counter we support is l2m_latency)
  {"dch", "mem_load_retired.l1_hit", REGNO_ANY, STXT ("L1 D-cache Hits"), PRELOADS_7, 0, ABST_NONE},
  {"dcm", "mem_load_retired.l1_miss", REGNO_ANY, STXT ("L1 D-cache Misses"), PRELOADS_65, 0, ABST_NONE},
  {"l2h", "mem_load_retired.l2_hit", REGNO_ANY, STXT ("L2 Cache Hits"), PRELOADS_65, 0, ABST_NONE},
  {"l2m", "mem_load_retired.l2_miss", REGNO_ANY, STXT ("L2 Cache Misses"), PRELOADS_6, 0, ABST_NONE},
  {"l2m_stall", "cycle_activity.stalls_l2_miss", REGNO_ANY, STXT ("L2 Cache Miss Stall"), PRELOADS_7, 1, ABST_NONE}, // needs validation
  {"l3h", "mem_load_retired.l3_hit", REGNO_ANY, STXT ("L3 Cache Hits"), PRELOADS_6, 0, ABST_NONE},
  {"l3m", "mem_load_retired.l3_miss", REGNO_ANY, STXT ("L3 Cache Misses"), PRELOADS_5, 0, ABST_NONE},
  {"l3m_stall", "cycle_activity.stalls_l3_miss", REGNO_ANY, STXT ("L3 Cache Miss Stall"), PRELOADS_7, 1, ABST_NONE}, // needs validation
  {"dtlbm_stall", "dtlb_load_misses.walk_active", REGNO_ANY, STXT ("DTLB Miss Est Stall"), PRELOADS_7, 1, ABST_NONE, STXT ("Estimated time stalled on DTLB misses requiring a tablewalk.  Does not include time related to STLB hits.")}, // needs validation
  // PEBS mem_inst_retired.stlb_miss_loads for finding location of DTLB issues
  // what about: dtlb_load_misses.walk_completed, dtlb_load_misses.walk_pending, dtlb_load_misses.stlb_hit

  {"fp_scalar", "fp_arith_inst_retired.scalar_double~umask=0x3", REGNO_ANY, STXT ("FP Scalar uOps"), PRELOADS_7, 0, ABST_NONE, STXT ("Floating-point scalar micro-ops that retired")},
  {"fp_vector", "fp_arith_inst_retired.128b_packed_double~umask=0x3c", REGNO_ANY, STXT ("FP Vector uOps"), /*needs test*/ PRELOADS_7, 0, ABST_NONE, STXT ("Floating-point vector micro-ops that retired")},

  // counters that can be time converted (add FFCs if we decide to support them)
  // counters that are load-store (did not include any... do we want to?)
  {/*08.10*/"dtlb_load_misses.walk_active", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*08.10*/"dtlb_load_misses.walk_pending", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0d.01*/"int_misc.recovery_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0d.01*/"int_misc.recovery_cycles_any", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0d.80*/"int_misc.clear_resteer_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0e.01*/"uops_issued.stall_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*14.01*/"arith.divider_active", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*3c.00*/"cpu_clk_unhalted.ring0_trans", NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {/*3c.00*/"cpu_clk_unhalted.thread_p_any", NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {/*3c.00*/"cpu_clk_unhalted.thread_p", NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {/*3c.00*/"cpu_clk_unhalted.core", NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {/*48.01*/"l1d_pend_miss.pending_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*48.01*/"l1d_pend_miss.pending_cycles_any", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*49.10*/"dtlb_store_misses.walk_active", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*49.10*/"dtlb_store_misses.walk_pending", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*4f.10*/"ept.walk_pending", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*5e.01*/"rs_events.empty_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60.01*/"offcore_requests_outstanding.cycles_with_demand_data_rd", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60.01*/"offcore_requests_outstanding.demand_data_rd_ge_6", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60.02*/"offcore_requests_outstanding.cycles_with_demand_code_rd", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60.04*/"offcore_requests_outstanding.cycles_with_demand_rfo", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60.08*/"offcore_requests_outstanding.cycles_with_data_rd", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60.10*/"offcore_requests_outstanding.cycles_with_l3_miss_demand_data_rd", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*60.10*/"offcore_requests_outstanding.l3_miss_demand_data_rd_ge_6", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*63.02*/"lock_cycles.cache_lock_duration", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.04*/"idq.mite_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.08*/"idq.dsb_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.10*/"idq.ms_dsb_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.18*/"idq.all_dsb_cycles_4_uops", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.18*/"idq.all_dsb_cycles_any_uops", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.24*/"idq.all_mite_cycles_4_uops", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.24*/"idq.all_mite_cycles_any_uops", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*79.30*/"idq.ms_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*80.04*/"icache_16b.ifdata_stall", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*83.04*/"icache_64b.iftag_stall", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*85.10*/"itlb_misses.walk_active", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*85.10*/"itlb_misses.walk_pending", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*87.01*/"ild_stall.lcp", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*9c.01*/"idq_uops_not_delivered.cycles_0_uops_deliv.core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*9c.01*/"idq_uops_not_delivered.cycles_le_1_uop_deliv.core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*9c.01*/"idq_uops_not_delivered.cycles_le_2_uop_deliv.core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*9c.01*/"idq_uops_not_delivered.cycles_le_3_uop_deliv.core", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*9c.01*/"idq_uops_not_delivered.cycles_fe_was_ok", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.01*/"uops_dispatched_port.port_0", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.02*/"uops_dispatched_port.port_1", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.04*/"uops_dispatched_port.port_2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.08*/"uops_dispatched_port.port_3", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.10*/"uops_dispatched_port.port_4", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.20*/"uops_dispatched_port.port_5", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.40*/"uops_dispatched_port.port_6", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a1.80*/"uops_dispatched_port.port_7", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2.01*/"resource_stalls.any", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a2.08*/"resource_stalls.sb", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a3.01*/"cycle_activity.cycles_l2_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a3.02*/"cycle_activity.cycles_l3_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a3.04*/"cycle_activity.stalls_total", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a3.05*/"cycle_activity.stalls_l2_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a3.06*/"cycle_activity.stalls_l3_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a3.08*/"cycle_activity.cycles_l1d_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a3.0c*/"cycle_activity.stalls_l1d_miss", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a3.10*/"cycle_activity.cycles_mem_any", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a3.14*/"cycle_activity.stalls_mem_any", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a6.01*/"exe_activity.exe_bound_0_ports", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a6.02*/"exe_activity.1_ports_util", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a6.04*/"exe_activity.2_ports_util", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a6.08*/"exe_activity.3_ports_util", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a6.10*/"exe_activity.4_ports_util", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a6.40*/"exe_activity.bound_on_stores", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a8.01*/"lsd.cycles_4_uops", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*a8.01*/"lsd.cycles_active", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*ab.02*/"dsb2mite_switches.penalty_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b1.01*/"uops_executed.cycles_ge_1_uop_exec", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b1.01*/"uops_executed.cycles_ge_2_uops_exec", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b1.01*/"uops_executed.cycles_ge_3_uops_exec", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b1.01*/"uops_executed.cycles_ge_4_uops_exec", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b1.01*/"uops_executed.stall_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b1.02*/"uops_executed.core_cycles_ge_1", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b1.02*/"uops_executed.core_cycles_ge_2", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b1.02*/"uops_executed.core_cycles_ge_3", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b1.02*/"uops_executed.core_cycles_ge_4", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*b1.02*/"uops_executed.core_cycles_none", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*c0.1*/"inst_retired.total_cycles_ps", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*c2.01*/"uops_retired.stall_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*c2.01*/"uops_retired.total_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*ca.1e*/"fp_assist.any", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  /* "Architectural" events: */
  {/* FFC */"cpu_clk_unhalted.thread", NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {/* FFC */"unhalted-core-cycles", NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  PERF_EVENTS_SW_EVENT_DEFS

  /* additional (hidden) aliases for convenience */
#if 0
  USE_INTEL_REF_CYCLES (25),
#endif
  {"insts0", "inst_retired.any_p", 0, NULL, PRELOADS_8, 0, ABST_NONE},
  {"insts1", "inst_retired.any_p", 1, NULL, PRELOADS_8, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

static Hwcentry intelLinuxUnknown[] = {
  PERF_EVENTS_SW_EVENT_ALIASES
  //    USE_INTEL_REF_CYCLES(100) // freq is unknown
  {"cycles", "unhalted-core-cycles", REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_75, 1, ABST_NONE},
  {"cycles", "PERF_COUNT_HW_CPU_CYCLES", REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_75, 1, ABST_NONE},
  {"insts", "instruction-retired", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_75, 0, ABST_NONE},
  {"insts", "PERF_COUNT_HW_INSTRUCTIONS", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_75, 0, ABST_NONE},

  {"dcm", "PERF_COUNT_HW_CACHE_MISSES.L1D", REGNO_ANY, STXT ("L1 D-cache Misses"), PRELOADS_65, 0, ABST_NONE},
  {"llm", "llc-misses", REGNO_ANY, STXT ("Last-Level Cache Misses"), PRELOADS_5, 0, ABST_NONE},
  {"llm", "PERF_COUNT_HW_CACHE_MISSES.LL", REGNO_ANY, STXT ("Last-Level Cache Misses"), PRELOADS_5, 0, ABST_NONE},

  {"br_msp", "branch-misses-retired", REGNO_ANY, STXT ("Branch Mispredict"), PRELOADS_6, 0, ABST_NONE},
  {"br_msp", "PERF_COUNT_HW_BRANCH_MISSES", REGNO_ANY, STXT ("Branch Mispredict"), PRELOADS_6, 0, ABST_NONE},
  {"br_ins", "branch-instruction-retired", REGNO_ANY, STXT ("Branch Instructions"), PRELOADS_7, 0, ABST_NONE},
  {"br_ins", "PERF_COUNT_HW_BRANCH_INSTRUCTIONS", REGNO_ANY, STXT ("Branch Instructions"), PRELOADS_7, 0, ABST_NONE},

  // counters that can be time converted (add FFCs if we decide to support them)
  // counters that are load-store (did not include any... do we want to?)
  /* "Architectural" events: */
  {/* FFC */"cpu_clk_unhalted.thread", NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {/* FFC */"unhalted-core-cycles", NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  PERF_EVENTS_SW_EVENT_DEFS

  /* additional (hidden) aliases for convenience */
  {"cycles0", "unhalted-reference-cycles", 0, NULL, PRELOADS_6, -(25), ABST_NONE}, //YXXX -can't do with ref cycles #
  {"cycles0", "PERF_COUNT_HW_BUS_CYCLES", 0, NULL, PRELOADS_6, -(25), ABST_NONE}, //YXXX -can't do with ref cycles #
  {"cycles1", "unhalted-reference-cycles", 1, NULL, PRELOADS_65, -(25), ABST_NONE}, //YXXX - can't do with ref cycles #
  {"cycles1", "PERF_COUNT_HW_BUS_CYCLES", 1, NULL, PRELOADS_65, -(25), ABST_NONE}, //YXXX - can't do with ref cycles #
  {"insts0", "instruction-retired", 0, NULL, PRELOADS_8, 0, ABST_NONE},
  {"insts0", "PERF_COUNT_HW_INSTRUCTIONS", 0, NULL, PRELOADS_8, 0, ABST_NONE},
  {"insts1", "instruction-retired", 1, NULL, PRELOADS_8, 0, ABST_NONE},
  {"insts1", "PERF_COUNT_HW_INSTRUCTIONS", 1, NULL, PRELOADS_8, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

static Hwcentry intelAtomList[] = {
  {"cycles", "cpu_clk_unhalted.core", /*6759307*/ REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_7, 1, ABST_NONE},
  {"cycles", "cpu_clk_unhalted.thread", /*6759307*/ REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_7, 1, ABST_NONE},
  {"insts", "instr_retired.any", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_7, 0, ABST_NONE},

  /* explicit definitions of (hidden) entries for proper counters */
  /*  Only counters that can be time converted, or are load-store need to be in this table */
  /* XXXX add core2-related entries if appropriate */
  {/*30A*/"cpu_clk_unhalted.core", /*6759307*/ NULL, REGNO_ANY, NULL, PRELOADS_7, 1, ABST_NONE},
  {/*30A*/"cpu_clk_unhalted.thread", /*6759307*/ NULL, REGNO_ANY, NULL, PRELOADS_7, 1, ABST_NONE},
  {/*0c*/"page_walks.cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*14*/"cycles_div_busy", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*21*/"l2_ads", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*22*/"l2_dbus_busy", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*32*/"l2_no_req", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*3c*/"cpu_clk_unhalted.core_p", NULL, REGNO_ANY, NULL, PRELOADS_7, 1, ABST_NONE},
  {/*3c*/"cpu_clk_unhalted.bus", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*3c*/"cpu_clk_unhalted.no_other", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*62*/"bus_drdy_clocks", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*63*/"bus_lock_clocks", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*64*/"bus_data_rcv", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*7a*/"bus_hit_drv", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*7b*/"bus_hitm_drv", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*7d*/"busq_empty", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*7e*/"snoop_stall_drv", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*7f*/"bus_io_wait", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*c6*/"cycles_int_masked.cycles_int_masked", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*c6*/"cycles_int_masked.cycles_int_pending_and_masked", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  /* "Architectural" events: */
  {/*3c*/"unhalted-core-cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  /* additional (hidden) aliases for convenience */
  {"cycles0", "cpu_clk_unhalted.core_p", 0, NULL, PRELOADS_75, 1, ABST_NONE},
  {"cycles1", "cpu_clk_unhalted.core_p", 1, NULL, PRELOADS_75, 1, ABST_NONE},
  {"insts0", "inst_retired.any_p", 0, NULL, PRELOADS_75, 0, ABST_NONE},
  {"insts1", "inst_retired.any_p", 1, NULL, PRELOADS_75, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

static Hwcentry amd_opteron_10h_11h[] = {
  {"cycles", "BU_cpu_clk_unhalted", REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_75, 1, ABST_NONE},
  {"insts", "FR_retired_x86_instr_w_excp_intr", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_75, 0, ABST_NONE},
  {"icr", "IC_fetch", REGNO_ANY, STXT ("L1 I-cache Refs"), PRELOADS_7, 0, ABST_NONE}, /* new */
  {"icm", "IC_miss", REGNO_ANY, STXT ("L1 I-cache Misses"), PRELOADS_6, 0, ABST_NONE},
  {"l2itlbh", "IC_itlb_L1_miss_L2_hit", REGNO_ANY, STXT ("L2 ITLB Hits"), PRELOADS_6, 0, ABST_NONE}, /* new */
  {"l2itlbm", "IC_itlb_L1_miss_L2_miss", REGNO_ANY, STXT ("L2 ITLB Misses"), PRELOADS_5, 0, ABST_NONE}, /* new */
  {"l2ir", "BU_internal_L2_req~umask=0x1", REGNO_ANY, STXT ("L2 I-cache Refs"), PRELOADS_6, 0, ABST_NONE},
  {"l2im", "BU_fill_req_missed_L2~umask=0x1", REGNO_ANY, STXT ("L2 I-cache Misses"), PRELOADS_4, 0, ABST_NONE},
  {"dcr", "DC_access", REGNO_ANY, STXT ("L1 D-cache Refs"), PRELOADS_7, 0, ABST_NONE}, /* new */
  {"dcm", "DC_miss", REGNO_ANY, STXT ("L1 D-cache Misses"), PRELOADS_65, 0, ABST_NONE}, /* new */
  {"l2dtlbh", "DC_dtlb_L1_miss_L2_hit", REGNO_ANY, STXT ("L2 DTLB Hits"), PRELOADS_6, 0, ABST_NONE}, /* new */
  {"l2dtlbm", "DC_dtlb_L1_miss_L2_miss", REGNO_ANY, STXT ("L2 DTLB Misses"), PRELOADS_5, 0, ABST_NONE}, /* new */
  {"l2dr", "BU_internal_L2_req~umask=0x2", REGNO_ANY, STXT ("L2 D-cache Refs"), PRELOADS_65, 0, ABST_NONE}, /* hwc_cache_load: 1.6x overcount on shanghai01 */
  {"l2dm", "BU_fill_req_missed_L2~umask=0x2", REGNO_ANY, STXT ("L2 D-cache Misses"), PRELOADS_6, 0, ABST_NONE}, /* new */
  {"fpadd", "FP_dispatched_fpu_ops~umask=0x1", REGNO_ANY, STXT ("FP Adds"), PRELOADS_7, 0, ABST_NONE},
  {"fpmul", "FP_dispatched_fpu_ops~umask=0x2", REGNO_ANY, STXT ("FP Muls"), PRELOADS_7, 0, ABST_NONE},
  {"fpustall", "FR_dispatch_stall_fpu_full", REGNO_ANY, STXT ("FPU Stall Cycles"), PRELOADS_7, 1, ABST_NONE},
  {"memstall", "FR_dispatch_stall_ls_full", REGNO_ANY, STXT ("Memory Unit Stall Cycles"), PRELOADS_7, 1, ABST_NONE},
  // For PAPI mappings, see hwctable.README.family10h
  // For PAPI mappings, see hwctable.README.opteron

  /* explicit definitions of (hidden) entries for proper counters */
  /*  Only counters that can be time converted, or are load-store need to be in this table */
  {"BU_cpu_clk_unhalted", NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {"FP_cycles_no_fpu_ops_retired", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"FP_serialize_ops_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"FR_dispatch_stall_branch_abort_to_retire", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_TBD},
  {"FR_dispatch_stall_far_ctl_trsfr_resync_branch_pend", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_TBD},
  {"FR_dispatch_stall_fpu_full", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_TBD},
  {"FR_dispatch_stall_ls_full", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_TBD},
  {"FR_dispatch_stall_reorder_buffer_full", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_TBD},
  {"FR_dispatch_stall_resv_stations_full", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_TBD},
  {"FR_dispatch_stall_segment_load", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_TBD},
  {"FR_dispatch_stall_serialization", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_TBD},
  {"FR_dispatch_stall_waiting_all_quiet", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_TBD},
  {"FR_dispatch_stalls", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_TBD},
  {"FR_intr_masked_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_TBD},
  {"FR_intr_masked_while_pending_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_TBD},
  {"FR_nothing_to_dispatch", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_TBD},
  {"IC_instr_fetch_stall", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_TBD},
  {"LS_buffer_2_full", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_TBD},
  {"NB_mem_ctrlr_dram_cmd_slots_missed", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {"NB_mem_ctrlr_turnaround", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_TBD},

  /* additional (hidden) aliases, for convenience */
  {"cycles0", "BU_cpu_clk_unhalted", 0, NULL, PRELOADS_8, 1, ABST_NONE},
  {"cycles1", "BU_cpu_clk_unhalted", 1, NULL, PRELOADS_8, 1, ABST_NONE},
  {"insts0", "FR_retired_x86_instr_w_excp_intr", 0, NULL, PRELOADS_8, 0, ABST_NONE},
  {"insts1", "FR_retired_x86_instr_w_excp_intr", 1, NULL, PRELOADS_8, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

static Hwcentry amd_15h[] = {
  {"cycles", "CU_cpu_clk_unhalted", REGNO_ANY, STXT ("CPU Cycles"), PRELOADS_75, 1, ABST_NONE},
  {"insts", "EX_retired_instr_w_excp_intr", REGNO_ANY, STXT ("Instructions Executed"), PRELOADS_75, 0, ABST_NONE},
  {"icr", "IC_fetch", REGNO_ANY, STXT ("L1 I-cache Refs"), PRELOADS_7, 0, ABST_NONE}, /* new */
  {"icm", "IC_miss", REGNO_ANY, STXT ("L1 I-cache Misses"), PRELOADS_6, 0, ABST_NONE},
  {"l2im", "IC_refill_from_system", REGNO_ANY, STXT ("L2 I-cache Misses"), PRELOADS_6, 0, ABST_NONE},
  {"dcr", "DC_access", REGNO_ANY, STXT ("L1 D-cache Refs"), PRELOADS_7, 0, ABST_NONE}, /* new */
  {"dcm", "DC_miss~umask=0x3", REGNO_ANY, STXT ("L1 D-cache Misses"), PRELOADS_65, 0, ABST_NONE}, /* new */
  {"l2dm", "DC_refill_from_system", REGNO_ANY, STXT ("L2 D-cache Misses"), PRELOADS_6, 0, ABST_NONE}, /* new */
  {"dtlbm", "DC_unified_tlb_miss~umask=0x7", REGNO_ANY, STXT ("L2 DTLB Misses"), PRELOADS_5, 0, ABST_NONE}, /* new */
  // For PAPI mappings, see hwctable.README.family15h

  /* explicit definitions of (hidden) entries for proper counters */
  /*  Only counters that can be time converted, or are load-store need to be in this table */
  {/*001.xx*/"FP_scheduler_empty", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*006.xx*/"FP_bottom_execute_uops_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*023.xx*/"LS_ldq_stq_full", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*024.xx*/"LS_locked_operation", /*umask!=0*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*069.xx*/"CU_mab_wait_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*076.xx*/"CU_cpu_clk_unhalted", NULL, REGNO_ANY, NULL, PRELOADS_75, 1, ABST_NONE},
  {/*087.xx*/"IC_instr_fetch_stall", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0cd.xx*/"EX_intr_masked_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0ce.xx*/"EX_intr_masked_while_pending_cycles", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0d0.xx*/"DE_nothing_to_dispatch", /*future*/ NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0d1.xx*/"DE_dispatch_stalls", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0d3.xx*/"DE_dispatch_stall_serialization", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0d5.xx*/"DE_dispatch_stall_instr_retire_q_full", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0d6.xx*/"DE_dispatch_stall_int_scheduler_q_full", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0d7.xx*/"DE_dispatch_stall_fp_scheduler_q_full", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0d8.xx*/"DE_dispatch_stall_ldq_full", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*0d9.xx*/"DE_dispatch_stall_waiting_all_quiet", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},
  {/*1d8.xx*/"EX_dispatch_stall_stq_full", NULL, REGNO_ANY, NULL, PRELOAD_DEF, 1, ABST_NONE},

  /* additional (hidden) aliases, for convenience */
  {"cycles0", "CU_cpu_clk_unhalted", 0, NULL, PRELOADS_8, 1, ABST_NONE},
  {"cycles1", "CU_cpu_clk_unhalted", 1, NULL, PRELOADS_8, 1, ABST_NONE},
  {"insts0", "EX_retired_instr_w_excp_intr", 0, NULL, PRELOADS_8, 0, ABST_NONE},
  {"insts1", "EX_retired_instr_w_excp_intr", 1, NULL, PRELOADS_8, 0, ABST_NONE},
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

#define USE_ARM_REF_CYCLES \
    {"usr_time","cycles",                 REGNO_ANY, STXT("User CPU"),   PRELOADS_85, 1, ABST_NONE}, \
    {"sys_time","cycles~system=1~user=0", REGNO_ANY, STXT("System CPU"), PRELOADS_85, 1, ABST_NONE}, \

static Hwcentry	armlist[] = {
  USE_ARM_REF_CYCLES
// Hardware event:
  {"branch-instructions",     NULL, REGNO_ANY, STXT("Branch-instructions"), PRELOADS_35, 0, ABST_NONE},
  {"branch-misses",           NULL, REGNO_ANY, STXT("Branch-misses"), PRELOADS_35, 0, ABST_NONE},
  {"bus-cycles",              NULL, REGNO_ANY, STXT("Bus Cycles"), PRELOADS_35, 1, ABST_NONE},
  {"cache-misses",            NULL, REGNO_ANY, STXT("Cache-misses"), PRELOADS_35, 0, ABST_NONE},
  {"cache-references",        NULL, REGNO_ANY, STXT("Cache-references"), PRELOADS_35, 0, ABST_NONE},
  {"cycles",                  NULL, REGNO_ANY, STXT("CPU Cycles"), PRELOADS_85, 1, ABST_NONE},
  {"insts",         "instructions", REGNO_ANY, STXT("Instructions Executed"), PRELOADS_75, 0, ABST_NONE},
  {"ref-cycles",              NULL, REGNO_ANY, STXT("Total Cycles"), PRELOADS_85, 1, ABST_NONE},
  {"stalled-cycles-backend",  NULL, REGNO_ANY, STXT("Stalled Cycles during issue."), PRELOADS_85, 1, ABST_NONE},
  {"stalled-cycles-frontend", NULL, REGNO_ANY, STXT("Stalled Cycles during retirement."), PRELOADS_85, 1, ABST_NONE},

// Software event:
  {"alignment-faults",        NULL, REGNO_ANY, STXT("Alignment Faults"), PRELOADS_85, 0, ABST_NONE},
  {"context-switches",        NULL, REGNO_ANY, STXT("Context Switches"), PRELOADS_85, 0, ABST_NONE},
  {"cpu-clock",               NULL, REGNO_ANY, STXT("CPU Clock"), PRELOADS_85, 1, ABST_NONE},
  {"cpu-migrations",          NULL, REGNO_ANY, STXT("CPU Migrations"), PRELOADS_85, 0, ABST_NONE},
  {"emulation-faults",        NULL, REGNO_ANY, STXT("Emulation Faults"), PRELOADS_85, 0, ABST_NONE},
  {"major-faults",            NULL, REGNO_ANY, STXT("Major Page Faults"), PRELOADS_85, 0, ABST_NONE},
  {"minor-faults",            NULL, REGNO_ANY, STXT("Minor Page Faults"), PRELOADS_85, 0, ABST_NONE},
  {"page-faults",             NULL, REGNO_ANY, STXT("Page Faults"), PRELOADS_85, 0, ABST_NONE},
  {"task-clock",              NULL, REGNO_ANY, STXT("Clock Count Specific"), PRELOADS_85, 1, ABST_NONE},

// Hardware cache event
  {"L1-dcache-load-misses",   NULL, REGNO_ANY, STXT("L1 D-cache Load Misses"), PRELOADS_35, 0, ABST_NONE},
  {"L1-dcache-loads",         NULL, REGNO_ANY, STXT("L1 D-cache Loads"), PRELOADS_35, 0, ABST_NONE},
  {"L1-dcache-store-misses",  NULL, REGNO_ANY, STXT("L1 D-cache Store Misses"), PRELOADS_35, 0, ABST_NONE},
  {"L1-dcache-stores",        NULL, REGNO_ANY, STXT("L1 D-cache Store Stores"), PRELOADS_35, 0, ABST_NONE},
  {"L1-icache-load-misses",   NULL, REGNO_ANY, STXT("L1 Instructions Load Misses"), PRELOADS_35, 0, ABST_NONE},
  {"L1-icache-load-misses",   NULL, REGNO_ANY, STXT("L1 Instructions Loads"), PRELOADS_35, 0, ABST_NONE},
  {"dTLB-load-misses",        NULL, REGNO_ANY, STXT("D-TLB Load Misses"), PRELOADS_35, 0, ABST_NONE},
  {"dTLB-loads",              NULL, REGNO_ANY, STXT("D-TLB Loads"), PRELOADS_35, 0, ABST_NONE},
  {"iTLB-load-misses",        NULL, REGNO_ANY, STXT("The Instruction TLB Load Misses"), PRELOADS_35, 0, ABST_NONE},
  {"iTLB-loads",              NULL, REGNO_ANY, STXT("The Instruction TLB Loads"), PRELOADS_35, 0, ABST_NONE},

  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

static Hwcentry unknownlist[] =
	/*  used for unrecognized CPU type */{
  {NULL, NULL, 0, NULL, 0, 0, 0, 0, ABST_NONE}
};

/* structure defining the counters for a CPU type */
typedef struct
{
  int cputag;
  Hwcentry *stdlist_table;
#define MAX_DEFAULT_HWC_DEFS 4 // allows multiple defs to handle OS variations; extend as needed
  char *default_exp_p[MAX_DEFAULT_HWC_DEFS + 1]; // end of list MUST be marked with NULL
} cpu_list_t;

/*  IMPORTANT NOTE:
 *
 *  Any default HWC string must consist of counter names separated by -TWO- commas,
 *  with a no trailing comma/value after the last counter name
 *
 *  Only aliased counters should be specified; non-aliased counters will
 *  not get the right overflow values set.
 *  If the string is not formatted that way, -h hi and -h lo will fail
 */
static cpu_list_t cputabs[] = {
  {CPC_ULTRA1, usIlist, {NULL}}, /* bind will fail */
  {CPC_ULTRA2, usIlist, {NULL}}, /* bind will fail */
  {CPC_ULTRA3, usIIIlist, {"insts,,ecstall", 0}},
  {CPC_ULTRA3_PLUS, usIIIlist, {"insts,,ecstall", 0}},
  {CPC_ULTRA3_I, usIIIlist, {"insts,,ecstall", 0}},
  {CPC_ULTRA4_PLUS, usIVplist, {"insts,,ecstall", 0}},
  {CPC_ULTRA_T1, niagara1, {"insts", 0}},
  {CPC_ULTRA_T2, niagara2, {"insts,,+l2drm", 0}},
  {CPC_ULTRA_T2P, niagara2, {"insts,,+l2drm", 0}},
  {CPC_ULTRA_T3, niagara2, {"insts,,+l2drm", 0}},
  {CPC_SPARC_T4, sparc_t4, {"insts,,cycles,,c_stalls,,dcm", "c_stalls", 0}},
  {CPC_SPARC_M4, sparc_t5_m6, {"insts,,cycles,,c_stalls,,dcm", "c_stalls", 0}}, // renamed to m5
  {CPC_SPARC_T5, sparc_t5_m6, {"insts,,cycles,,c_stalls,,dcm", "c_stalls", 0}},
  {CPC_SPARC_M5, sparc_t5_m6, {"insts,,cycles,,c_stalls,,dcm", "c_stalls", 0}},
  {CPC_SPARC_T6, sparc_t5_m6, {"insts,,cycles,,c_stalls,,dcm", "c_stalls", 0}}, // no such processor
  {CPC_SPARC_M6, sparc_t5_m6, {"insts,,cycles,,c_stalls,,dcm", "c_stalls", 0}},
  {CPC_SPARC_M7, sparc_m7, {"insts,,cycles,,c_stalls,,dcm", "c_stalls", 0}}, // includes T7
  {CPC_SPARC_M8, sparc_m8, {"insts,,cycles,,c_stalls,,dcm", "c_stalls", 0}},
  {CPC_PENTIUM_PRO_MMX, pentiumIIlist, {"insts", 0}},
  {CPC_PENTIUM_PRO, pentiumIIIlist, {"insts", 0}},
  {CPC_PENTIUM_4, pentium4, {"insts", 0}},
  {CPC_PENTIUM_4_HT, pentium4, {"insts", 0}},
  {CPC_INTEL_CORE2, intelCore2list, {"insts,,cycles", 0}},
  {CPC_INTEL_NEHALEM, intelNehalemList, {"insts,,cycles,,+l2m_latency,,dtlbm_stall",
      "insts,,cycles,,l3m_stall,,dtlbm_stall", 0}},
  {CPC_INTEL_WESTMERE, intelNehalemList, {"insts,,cycles,,+l2m_latency,,dtlbm_stall",
      "insts,,cycles,,l3m_stall,,dtlbm_stall", 0}},
  {CPC_INTEL_SANDYBRIDGE, intelSandyBridgeList, {"insts,,cycles,,+l2m_latency,,dtlbm_stall",
      "insts,,cycles,,l3m,,dtlbm", 0}},
  {CPC_INTEL_IVYBRIDGE, intelSandyBridgeList, {"insts,,cycles,,+l2m_latency,,dtlbm_stall",
      "insts,,cycles,,l3m,,dtlbm", 0}},
  {CPC_INTEL_HASWELL, intelHaswellList, {"insts,,cycles,,+l2m_latency,,dtlbm_stall",
      "insts,,cycles,,l3m,,dtlbm", 0}},
  {CPC_INTEL_BROADWELL, intelBroadwellList, {"insts,,cycles,,+l2m_latency,,dtlbm",
      "insts,,cycles,,l3m,,dtlbm", 0}},
  {CPC_INTEL_SKYLAKE, intelSkylakeList, {"insts,,cycles,,+l2m_latency,,dtlbm_stall",
      "insts,,cycles,,l2m_stall,,dtlbm_stall", 0}},
  {CPC_INTEL_UNKNOWN, intelLinuxUnknown, {"cycles,,insts,,llm",
      "user_time,,system_time,,cycles,,insts,,llm", 0}},
  {CPC_INTEL_ATOM, intelAtomList, {"insts", 0}},
  {CPC_AMD_K8C, amd_opteron_10h_11h, {"insts,,cycles,,l2dm,,l2dtlbm", 0}},
  {CPC_AMD_FAM_10H, amd_opteron_10h_11h, {"insts,,cycles,,l2dm,,l2dtlbm", 0}},
  {CPC_AMD_FAM_11H, amd_opteron_10h_11h, {"insts,,cycles,,l2dm,,l2dtlbm", 0}},
  {CPC_AMD_FAM_15H, amd_15h, {"insts,,cycles", 0}},
  {CPC_SPARC64_V, usfuji_V_list, {"insts,,cycles", 0}},
  {CPC_SPARC64_VI, usfuji_VI_VII_list, {"insts,,cycles,,dcstall", 0}},
  {CPC_SPARC64_VII, usfuji_VI_VII_list, {"insts,,cycles,,dcstall", 0}},
  {CPC_SPARC64_X, usfuji_X_list, {"insts,,cycles,,dcstall", 0}},
  {CPC_SPARC64_XII, usfuji_XII_list, {"insts,,cycles,,dcstall", 0}},
  {CPC_KPROF, kproflist, {NULL}}, // OBSOLETE (To support 12.3 and earlier, TBR)
  {ARM_CPU_IMP_APM, armlist, {"insts,,cycles", 0}},
  {0, unknownlist, {NULL}} /* processor is unknown, but experiment is allowed */
};

/*---------------------------------------------------------------------------*/
/* state variables */
static int initialized;
static int signals_disabled;

// Simple array list
typedef struct
{
  void** array;     // array of ptrs, last item set to null
  int sz;           // num live elements in array
  int max;          // array allocation size
} ptr_list;

static void
ptr_list_init (ptr_list *lst)
{
  lst->sz = 0;
  lst->max = 0;
  lst->array = 0;
}

static void
ptr_list_add (ptr_list *lst, char* ptr)
{ // ptr must be freeable
  if (lst->sz >= lst->max - 1)
    {
      void * * new;
      int newmax = lst->max ? lst->max * 2 : 16;
      new = (void**) realloc (lst->array, newmax * sizeof (void*));
      if (!new) return; // failed, discard add
      lst->max = newmax;
      lst->array = new;
    }
  lst->array[lst->sz++] = ptr;
  lst->array[lst->sz] = NULL; // mark new end-of-list
}

static void
ptr_list_free (ptr_list *lst)
{ // includes shallow free of all elements
  if (lst->array)
    {
      for (int ii = 0; lst->array[ii]; ii++)
	free (lst->array[ii]);
      free (lst->array);
    }
  lst->sz = 0;
  lst->max = 0;
  lst->array = 0;
}

// Capabilities of this machine (initialized by setup_cpc())
static int cpcx_cpuver = CPUVER_UNDEFINED;
static uint_t cpcx_npics;
static const char *cpcx_cciname;
static const char *cpcx_docref;
static uint64_t cpcx_support_bitmask;

// cpcx_*[0]: collect lists
// cpcx_*[1]: er_kernel lists
// Each cpcx_*[] list is an array of ptrs with null ptr marking end of list
static char **cpcx_attrs[2];

static Hwcentry **cpcx_std[2];
static Hwcentry **cpcx_raw[2];
static Hwcentry **cpcx_hidden[2];

static uint_t cpcx_max_concurrent[2];
static char *cpcx_default_hwcs[2];
static char *cpcx_orig_default_hwcs[2];
static int cpcx_has_precise[2];

#define VALID_FOR_KERNEL(forKernel) ((forKernel)>=0 && (forKernel)<=1)
#define IS_KERNEL(forKernel) ((forKernel)==1)

// used to build lists:
static ptr_list unfiltered_attrs;
static ptr_list unfiltered_raw;

/*---------------------------------------------------------------------------*/
/* misc internal utilities */

/* compare 2 strings to either \0 or <termchar> */
#define IS_EOL(currchar, termchar) ((currchar)==(termchar) || (currchar)==0)

static int
is_same (const char * regname, const char * int_name, char termchar)
{
  do
    {
      char a = *regname;
      char b = *int_name;
      if (IS_EOL (a, termchar))
	{
	  if (IS_EOL (b, termchar))
	    return 1; /* strings are the same up to terminating char */
	  else
	    break; /* strings differ */
	}
      if (a != b)
	break;      /* strings differ */
      regname++;
      int_name++;
    }
  while (1);
  return 0;
}

static int
is_numeric (const char *name, uint64_t *pval)
{
  char *endptr;
  uint64_t val = strtoull (name, &endptr, 0);
  if (!*name || *endptr)
    return 0; /* name does not specify a numeric value */
  if (pval)
    *pval = val;
  return 1;
}

static int
is_visible_alias (Hwcentry* pctr)
{
  if (!pctr)
    return 0;
  if (pctr->name && pctr->int_name && pctr->metric)
    return 1;
  return 0;
}

static int
is_hidden_alias (Hwcentry* pctr)
{
  if (!pctr)
    return 0;
  if (pctr->name && pctr->int_name && pctr->metric == NULL)
    return 1;
  return 0;
}

static int
is_numeric_alias (Hwcentry* pctr)
{
  int is_numeric_alias = 0;
  regno_t regno;
  char *nameOnly = NULL;
  hwcfuncs_parse_ctr (pctr->int_name, NULL, &nameOnly, NULL, NULL, &regno);
  if (is_numeric (nameOnly, NULL))
    is_numeric_alias = 1;
  free (nameOnly);
  return is_numeric_alias;
}

/* print list of register to a buffer */
/*
 *  style      e x a m p l e s
 *    0        NONE    2       {0|1|2|3}
 *    1        NONE    2       : 0, 1, 2, or 3
 *    2                        0 1 2 3     6
 */
static char *
get_regnolist (char *buf, size_t sz, const regno_t *reg_list, int style)
{
  if (!buf || !sz)
    return "INTERNAL ERROR";
  buf[0] = 0;
  if (style == 2)
    {
      int ii;
      // width should be consistent with that in format_columns()
      // the format will accommodate cpcx_npics regs
      if (cpcx_npics < 1)
	return "INTERNAL ERROR";
      // clear out the buffer
      for (ii = 0; ii < sz; ii++)
	buf[ii] = '_';
      if (cpcx_npics <= 9)
	{
	  // one char per reg, plus terminating null char
	  if (cpcx_npics + 1 > sz)
	    return "INTERNAL ERROR";
	  buf[cpcx_npics] = '\0';

	  // fill buf with regnos
	  for (ii = 0; ii < MAX_PICS; ii++)
	    {
	      regno_t regno = reg_list[ii];
	      if (REG_LIST_EOL (regno))
		break;
	      if (regno < 0 || regno >= cpcx_npics)
		return "INTERNAL ERROR";
	      buf[regno] = '0' + regno;
	    }
	}
      else
	{
	  /* space between regs, which may be 1 or 2 digits each
	   *   1 char  for reg 0
	   *   2 chars for regs 1-9 each
	   *   3 chars for regs 10- each
	   *   1 char  for terminating null char
	   */
	  int nchars = 17 + 3 * (cpcx_npics - 9);
	  if (nchars > sz)
	    return "INTERNAL ERROR";
	  buf[nchars - 1] = '\0';

	  // fill buf with regnos
	  for (ii = 0; ii < MAX_PICS; ii++)
	    {
	      regno_t regno = reg_list[ii];
	      if (REG_LIST_EOL (regno))
		break;
	      if (regno <= 9)
		buf[2 * regno ] = '0' + regno;
	      else
		{
		  buf[3 * (regno - 9) + 17] = '0' + (regno / 10);
		  buf[3 * (regno - 9) + 18] = '0' + (regno % 10);
		}
	    }
	}
      return buf;
    }
  if (REG_LIST_IS_EMPTY (reg_list))
    {
      snprintf (buf, sz, GTXT ("NONE"));
      return buf;
    }
  else if (REG_LIST_EOL (reg_list[1]))
    {
      /* 1 item in list */
      snprintf (buf, sz, "%d", reg_list[0]);
      return buf;
    }
  else
    {
      /* 2 more items in list */
      int ii, num_regs;
      for (ii = 0; ii < MAX_PICS; ii++)
	{
	  regno_t regno = reg_list[ii];
	  if (REG_LIST_EOL (regno))
	    break;
	}
      num_regs = ii;
      buf[0] = 0;
      for (ii = 0; ii < num_regs; ii++)
	{
	  regno_t regno = reg_list[ii];
	  if (style == 0)
	    snprintf (buf + strlen (buf), sz - strlen (buf),
		      "%c%d", ii ? '|' : '{', regno);
	  else
	    {
	      if (num_regs == 2)
		snprintf (buf + strlen (buf), sz - strlen (buf),
			  "%d%s", regno, !ii ? " or " : "");
	      else
		{
		  /* 3 or more items in list */
		  if (ii < num_regs - 2)
		    snprintf (buf + strlen (buf), sz - strlen (buf),
			      "%d, ", regno);
		  else if (ii == num_regs - 2)
		    snprintf (buf + strlen (buf), sz - strlen (buf),
			      "%d, or ", regno);
		  else
		    snprintf (buf + strlen (buf), sz - strlen (buf),
			      "%d", regno);
		}
	    }
	}
      if (style == 0)
	snprintf (buf + strlen (buf), sz - strlen (buf), "}");
    }
  return buf;
}

#if !HWC_DEBUG
#define hwcentry_print(lvl,x1,x2)
#else

/* print a Hwcentry */
static void
hwcentry_print (int lvl, const char * header, const Hwcentry *pentry)
{
  char buf[1024];
  Tprintf (lvl, "%s '%s', '%s', %d, '%s', %d, %d, %d, %d, %d, %d, /",
	   header,
	   pentry->name ? pentry->name : "NULL",
	   pentry->int_name ? pentry->int_name : "NULL",
	   pentry->reg_num,
	   pentry->metric ? pentry->metric : "NULL",
	   pentry->lval, /* low-resolution/long run */
	   pentry->val, /* normal */
	   pentry->hval, /* high-resolution/short run */
	   pentry->timecvt,
	   pentry->memop, /* type of instruction that can trigger */
	   pentry->sort_order);
  get_regnolist (buf, sizeof (buf), pentry->reg_list, 0);
  Tprintf (lvl, "%s\n", buf);
}
#endif

/* add <regno> to a Hwcentry's list */
static void
regno_add (Hwcentry * pctr, regno_t regno)
{
  int jj;
  regno_t *reg_list;
  if (!pctr)
    {
      Tprintf (0, "hwctable: regno_add(): ERROR: pctr==NULL\n");
      return;
    }
  reg_list = pctr->reg_list;
  if (!reg_list)
    {
      /* create list */
      reg_list = (regno_t*) malloc (sizeof (regno_t*) * MAX_PICS);
      if (!reg_list)
	{
	  hwcentry_print (DBG_LT0, "hwctable: regno_add: ERROR:"
			  " Out of memory: ", pctr);
	  return;
	}
      /* initialize list */
      for (jj = 0; jj < MAX_PICS; jj++)
	reg_list[jj] = REGNO_ANY;
      pctr->reg_list = reg_list;
    }
  if (regno == REGNO_ANY)
    {
      /* add all counters up to cpcx_npics */
      for (jj = 0; jj < MAX_PICS && jj < cpcx_npics; jj++)
	reg_list[jj] = jj;
    }
  else
    {
      /* add <regno> to list of registers */
      for (jj = 0; jj < MAX_PICS; jj++)
	{
	  if (reg_list[jj] == regno)
	    {
	      hwcentry_print (DBG_LT0, "hwctable: regno_add: WARNING: "
			      "Duplicate regno: ", pctr);
	      break;
	    }
	  if (reg_list[jj] == REGNO_ANY)
	    {
	      reg_list[jj] = regno;
	      break;
	    }
	}
    }
  if (jj == MAX_PICS)
    hwcentry_print (DBG_LT0, "hwctable: regno_add: WARNING:"
		    " regno list is full:", pctr);
}

/*---------------------------------------------------------------------------*/
/* utilities for rawlist (list of raw counters with reglist[] filled in) */

/* search the 'raw' list of counters for <name> */
static Hwcentry *
ptrarray_find_by_name (Hwcentry** array, const char * name)
{
  if (name == NULL)
    return NULL;
  Tprintf (DBG_LT3, "hwctable: array_find_by_name(%s):\n", name);
  for (int ii = 0; array && array[ii]; ii++)
    if (strcmp (array[ii]->name, name) == 0)
      return array[ii];
  return NULL; /* not found */
}

/* add Hwcentry to the 'raw' list of counters */
static Hwcentry *
alloc_shallow_copy (const Hwcentry *pctr)
{
  Hwcentry *node = (Hwcentry *) malloc (sizeof (Hwcentry));
  if (!node)
    return NULL; // fail
  *node = *pctr; /* shallow copy! */
  if (pctr->name)
    node->name = strdup (pctr->name);
  return node;
}

/* add Hwcentry to the 'raw' list of counters */
static Hwcentry *
list_append_shallow_copy (ptr_list *list, const Hwcentry *pctr)
{
  Hwcentry *node = alloc_shallow_copy (pctr);
  if (!node)
    return NULL; // fail
  ptr_list_add (list, (void*) node);
  return node;
}

static Hwcentry *
list_add (ptr_list *list, uint_t regno, const char *name)
{
  Hwcentry *praw;
  praw = ptrarray_find_by_name ((Hwcentry**) list->array, name);
  if (!praw)
    {
      Hwcentry tmpctr = empty_ctr;
      tmpctr.name = (char *) name;
      praw = list_append_shallow_copy (list, &tmpctr);
    }
  if (praw)
    regno_add (praw, regno);
  return praw;
}

/*---------------------------------------------------------------------------*/
/* utilities for stdlist (table of aliased, hidden, & convenience, ctrs) */

/* find top level definition for <cpuid> */
static cpu_list_t*
cputabs_find_entry (int cpuid)
{
  int i;
  /* now search for the appropriate table */
  for (i = 0;; i++)
    {
      if (cputabs[i].cputag == 0)
	break;
      if (cpuid == cputabs[i].cputag)
	return &cputabs[i];
    }
  Tprintf (0, "hwctable: cputabs_find_entry: WARNING: "
	   "cpu_id = %d not defined.  No 'standard' counters are available\n",
	   cpuid);
  return &cputabs[i];
}

/* find Hwcentry table for <cpuid> */
static Hwcentry*
stdlist_get_table (int cpuid)
{
  cpu_list_t* tmp = cputabs_find_entry (cpuid);
  if (tmp)
    return tmp->stdlist_table;
  return NULL;
}

/* search the 'standard' list of counters for <name>,<regno> */
/* note: <regno>=REGNO_ANY is a wildcard that matches any value. */

/* note: int_name==NULL is a wildcard */
static const Hwcentry *
ptrarray_find (const Hwcentry **array, const char *name, const char *int_name,
	       int check_regno, regno_t regno)
{
  const Hwcentry *pctr;
  if (!array)
    return NULL;
  for (int ii = 0; array[ii]; ii++)
    {
      pctr = array[ii];
      if (strcmp (pctr->name, name))
	continue;
      if (int_name && int_name[0] != 0 && pctr->int_name)
	{
	  if (NULL == strstr (int_name, pctr->int_name))
	    continue;
	}
      if (!check_regno)
	return pctr;
      else
	{
	  /* duplicates aliases are allowed in table because of 6759307 */
	  if (REG_LIST_IS_EMPTY (pctr->reg_list))
	    {
	      /* skip aliases that don't have a valid list of registers */
	      hwcentry_print (1, "hwctable: stdlist_find_by_name:"
			      " WARNING: alias found, but event not supported by HW:",
			      pctr);
	      continue;
	    }
	  if (!regno_is_valid (pctr, regno))
	    {
	      hwcentry_print (1, "hwctable: stdlist_find_by_name():"
			      " WARNING: alias found, but regno doesn't match:",
			      pctr);
	      continue;
	    }
	  return pctr;
	}
    }
  return NULL;
}

/* search the 'standard' list of counters for <name>,<regno> */

/* note: <regno>=REGNO_ANY is a wildcard that matches any value. */
static const Hwcentry *
static_table_find (const Hwcentry *table, const char *name, const char *int_name,
		   int check_regno, regno_t regno)
{
  int sz;
  for (sz = 0; table && table[sz].name; sz++)
    ;
  if (!sz)
    return NULL;
  const Hwcentry ** list = calloc (sz + 1, sizeof (void*));
  if (!list)
    return NULL;
  for (int ii = 0; ii < sz; ii++)
    list[ii] = &table[ii];
  list[sz] = NULL;
  const Hwcentry *pctr = ptrarray_find (list, name, int_name, check_regno, regno);
  free (list);
  return pctr;
}

#if !HWC_DEBUG
#define stdlist_print(dbg_lvl,table)
#else

/* print all Hwcentries in standard table.  Check for weird stuff */
static void
stdlist_print (int dbg_lvl, const Hwcentry* table)
{
  const Hwcentry *pctr;
  if (!table)
    {
      Tprintf (0, "hwctable: stdlist_print: ERROR: "
	       "table is invalid.\n");
      return;
    }
  for (pctr = table; pctr->name; pctr++)
    {
      int ii;
      hwcentry_print (dbg_lvl, "hwctable: stdlist: ", pctr);
      if (REG_LIST_IS_EMPTY (pctr->reg_list))
	{
	  if (pctr->int_name || !pctr->metric)
	    hwcentry_print (DBG_LT1, "hwctable: stdlist_print: WARNING: "
			    "no hardware event found for table entry", pctr);
	  continue;
	}
      /* check if incorrect reg_num used in table */
      if (!regno_is_valid (pctr, pctr->reg_num))
	{
	  hwcentry_print (DBG_LT0, "hwctable: stdlist_print: ERROR: "
			  "reg_num is not in table. ", pctr);
	  continue;
	}
      for (ii = 0; ii < MAX_PICS; ii++)
	{
	  regno_t regno = pctr->reg_list[ii];
	  if (REG_LIST_EOL (regno))
	    break;
	}
      if (ii > 1 && pctr->reg_num != REGNO_ANY)
	{
	  /* several regnos were valid, but only one can be specified */
	  if (pctr->metric || !pctr->int_name)
	    {
	      /* pctr is standard or a raw definition */
	      /* (pctr is not an alias like cycles0) */
	      hwcentry_print (DBG_LT0, "hwctable: stdlist_print: ERROR: "
			      "regno in table should have been REGNO_ANY. ",
			      pctr);
	    }
	}
    }
}
#endif

/*---------------------------------------------------------------------------*/
/* utilities for init */

/* try to bind counters to hw.  Return 0 on success, nonzero otherwise */
static int
test_hwcs (const Hwcentry* entries[], unsigned numctrs)
{
  int rc = -1;
  hwc_event_t sample;
  int created = 0;
  hwcdrv_api_t *hwcdrv = get_hwcdrv ();
  Tprintf (DBG_LT2, "hwctable: test_hwcs()...\n");
  rc = hwcfuncs_bind_hwcentry (entries, numctrs);
  if (rc)
    {
      Tprintf (0, "hwctable: WARNING: test "
	       "counters could not be created\n");
      goto end_test_hwcs;
    }
  created = 1;
  if (!signals_disabled)
    {
      (void) signal (HWCFUNCS_SIGNAL, SIG_IGN);
      signals_disabled = 1;
    }
  rc = hwcdrv->hwcdrv_start ();
  if (rc)
    {
      Tprintf (0, "hwctable: WARNING: test "
	       "counters could not be started\n");
      goto end_test_hwcs;
    }
  rc = hwcdrv->hwcdrv_read_events (&sample, NULL);
  if (rc)
    Tprintf (0, "hwctable: WARNING: test sample failed\n");
  rc = 0;
#if HWC_DEBUG
  {
    unsigned ii;
    Tprintf (DBG_LT1, "hwctable: test_hwcs(");
    for (ii = 0; ii < numctrs; ii++)
      Tprintf (DBG_LT1, "%s%s", ii ? "," : "", entries[ii]->name);
    Tprintf (DBG_LT1, ") PASS\n");
  }
#endif

end_test_hwcs:
  if (created && hwcdrv->hwcdrv_free_counters ())
    Tprintf (0, "hwctable: WARNING: test counters could not be freed\n");
  return rc;
}

#if !HWC_DEBUG
#define check_tables()
#else

/* check for typos in tables */
static void
check_tables ()
{
  int i;
  /* now search the known table of counters */
  for (i = 0;; i++)
    {
      Hwcentry * pentry;
      int cputag = cputabs[i].cputag;
      if (cputag == 0)
	break;
      if (cputag == CPC_KPROF)
	continue;
      pentry = cputabs[i].stdlist_table;
      for (; pentry; pentry++)
	{
	  if (!pentry->name)
	    break;
	  if (!pentry->int_name)
	    {/* internal, only to supply ABST and timecvt */
	      if (pentry->metric)
		Tprintf (DBG_LT0, "hwctable: check_tables: ERROR:"
			 " internal && metric @%d, %s\n", cputag, pentry->name);
	      if (pentry->reg_num != REGNO_ANY)
		Tprintf (DBG_LT1, "hwctable: check_tables: WARNING:"
			 " internal && reg_num!=REGNO_ANY @%d, %s\n",
			 cputag, pentry->name);
	      if (pentry->val != PRELOAD_DEF
		  && pentry->memop != ABST_EXACT_PEBS_PLUS1)
		Tprintf (DBG_LT2, "hwctable: check_tables: INFO:"
			 " internal && custom val=%d @%d, %s\n",
			 pentry->val, cputag, pentry->name);
#if 0
	      if (!pentry->timecvt && pentry->memop == ABST_NONE)
		Tprintf (DBG_LT0, "hwctable: check_tables: ERROR:"
			 " internal && not special! @%d, %s\n",
			 cputag, pentry->name);
#endif
	    }
	  if (pentry->metric)
	    { /* aliased */
	      if (!pentry->int_name)
		Tprintf (DBG_LT0, "hwctable: check_tables: ERROR:"
			 " aliased && !int_name @%d, %s\n", cputag, pentry->name);
#if 0
	      else if (!strcmp (pentry->name, pentry->int_name))
		Tprintf (DBG_LT0, "hwctable: check_tables: ERROR:"
			 " name==int_name @%d, %s\n",
			 cputag, pentry->name);
#endif
	      if (pentry->reg_num != REGNO_ANY && pentry->reg_num != REGNO_INVALID)
		Tprintf (DBG_LT1, "hwctable: check_tables: INFO:"
			 " aliased && custom reg_num==%d @%d, %s\n",
			 pentry->reg_num, cputag, pentry->name);
	      if (pentry->reg_num == REGNO_INVALID)
		Tprintf (DBG_LT2, "hwctable: check_tables: INFO:"
			 " aliased && reg_num==REGNO_INVALID @%d, %s\n",
			 cputag, pentry->name);
	    }
	  if (pentry->int_name && !pentry->metric)
	    { /* convenience */
	      if (!strcmp (pentry->name, pentry->int_name))
		  Tprintf (DBG_LT0, "hwctable: check_tables: ERROR:"
			   " convenience && name==int_name @%d, %s\n",
			   cputag, pentry->name);
	      if (pentry->reg_num == REGNO_ANY)
		  Tprintf (DBG_LT0, "hwctable: check_tables: ERROR:"
			   " convenience && reg_num==REGNO_ANY @%d, %s\n",
			   cputag, pentry->name);
	    }
	}
    }
}
#endif

static int try_a_counter ();
static void hwc_process_raw_ctrs (int forKernel, Hwcentry ***pstd_out,
				  Hwcentry ***praw_out, Hwcentry ***phidden_out,
				  Hwcentry**static_tables,
				  Hwcentry **raw_unfiltered_in);

/* internal call to initialize libs, ctr tables */
static void
setup_cpc_general (int skip_hwc_test)
{
  const cpu_list_t* cputabs_entry;
  int rc = -1;
  Tprintf (DBG_LT2, "hwctable: setup_cpc()... \n");
  if (initialized)
    {
      Tprintf (0, "hwctable: WARNING: setup_cpc() has already been called\n");
      return;
    }
  initialized = 1;
  cpcx_cpuver = CPUVER_UNDEFINED;
  cpcx_cciname = NULL;
  cpcx_npics = 0;
  cpcx_docref = NULL;
  cpcx_support_bitmask = 0;
  for (int kk = 0; kk < 2; kk++)
    { // collect-0 and kernel-1
      cpcx_attrs[kk] = NULL;
      cpcx_std[kk] = NULL;
      cpcx_raw[kk] = NULL;
      cpcx_hidden[kk] = NULL;
      cpcx_max_concurrent[kk] = 0;
      cpcx_default_hwcs[kk] = NULL;
      cpcx_orig_default_hwcs[kk] = NULL;
      cpcx_has_precise[kk] = 0;
    }
  check_tables ();
  hwcdrv_api_t *hwcdrv = get_hwcdrv ();
  if (hwcdrv->hwcdrv_init_status)
    {
      Tprintf (0, "WARNING: setup_cpc_general() failed. init_status=%d \n",
	       hwcdrv->hwcdrv_init_status);
      goto setup_cpc_wrapup;
    }
  hwcdrv->hwcdrv_get_info (&cpcx_cpuver, &cpcx_cciname, &cpcx_npics,
			   &cpcx_docref, &cpcx_support_bitmask);

#ifdef DISALLOW_USI_USII_6357446
  if (cpcx_cpuver == CPC_ULTRA1 || cpcx_cpuver == CPC_ULTRA2)
    {
      Tprintf (0, "hwctable: WARNING: setup_cpc(): cpu=%d"
	       " US-I/US-II cannot provide profile interrupts\n", cpcx_cpuver);
      /* profiling interrupts don't work on US-I, US-II */
      hwcfuncs_int_logerr (GTXT ("UltraSPARC I and II cannot provide overflow interrupts\n"));
      goto setup_cpc_wrapup;
    }
#endif

#ifdef DISALLOW_PENTIUM_PRO_MMX_7007575
  if (cpcx_cpuver == CPC_PENTIUM_PRO_MMX)
    {
      Tprintf (0, "hwctable: WARNING: setup_cpc(): cpu=%d"
	       " `Pentium Pro with MMX, Pentium II' is not supported\n", cpcx_cpuver);
      hwcfuncs_int_logerr (GTXT ("libcpc cannot identify processor type\n"));
      goto setup_cpc_wrapup;
    }
#endif

  /* now search the known table of counters */
  cputabs_entry = cputabs_find_entry (cpcx_cpuver);
  if (cputabs_entry == NULL)
    {
      Tprintf (0, "hwctable: WARNING: setup_cpc(): cpu=%d"
	       " could not be found in the tables\n", cpcx_cpuver);
      /* strange, should have at least selected "unknownlist" */
      hwcfuncs_int_logerr (GTXT ("Analyzer CPU table could not be found\n"));
      goto setup_cpc_wrapup;
    }

  Hwcentry * valid_cpu_tables[2]; // [0]:static table of counters, [1]:static table of generic counters
  valid_cpu_tables[0] = cputabs_entry->stdlist_table;
  if (valid_cpu_tables[0] == NULL)
    {
      Tprintf (0, "hwctable: WARNING: setup_cpc(): "
	       " valid_cpu_tables was NULL??\n");
      /* strange, someone put a NULL in the lookup table? */
      hwcfuncs_int_logerr (GTXT ("Analyzer CPU table is invalid\n"));
      goto setup_cpc_wrapup;
    }
  valid_cpu_tables[1] = papi_generic_list;
  Tprintf (DBG_LT2, "hwctable: setup_cpc(): getting descriptions \n");
  // populate cpcx_raw and cpcx_attr
  hwcdrv->hwcdrv_get_descriptions (hwc_cb, attrs_cb);
  for (int kk = 0; kk < 2; kk++)
    { // collect and er_kernel
      hwc_process_raw_ctrs (kk, &cpcx_std[kk], &cpcx_raw[kk], &cpcx_hidden[kk],
			    valid_cpu_tables, (Hwcentry**) unfiltered_raw.array);
      cpcx_has_precise[kk] = 0;
      for (int rr = 0; cpcx_raw[kk] && cpcx_raw[kk][rr]; rr++)
	{
	  int memop = cpcx_raw[kk][rr]->memop;
	  if (ABST_MEMSPACE_ENABLED (memop))
	    {
	      cpcx_has_precise[kk] = 1;
	      break;
	    }
	}
      cpcx_attrs[kk] = (char**) unfiltered_attrs.array;
      cpcx_max_concurrent[kk] = cpcx_npics;
    }
#if 1 // 22897042 - DTrace cpc provider does not support profiling on multiple ctrs on some systems
  if ((cpcx_support_bitmask & HWCFUNCS_SUPPORT_OVERFLOW_CTR_ID) != HWCFUNCS_SUPPORT_OVERFLOW_CTR_ID)
    {
      // kernel profiling only supports one counter if overflowing counter can't be identified
      cpcx_max_concurrent[1] = cpcx_npics ? 1 : 0;
    }
#endif

  /* --- quick test of the cpc interface --- */
  if (skip_hwc_test)
    rc = 0;
  else
    rc = try_a_counter (0);

  /* initialize the default counter string definition */
  for (int kk = 0; kk < 2; kk++)
    {
      char * default_exp = 0;
      int jj;
      for (jj = 0; (default_exp = cputabs_entry->default_exp_p[jj]); jj++)
	{
	  int rc = hwc_lookup (kk, 0, default_exp, NULL, 0, NULL, NULL);
	  if (rc > 0)
	    break;
	}
      if (!default_exp)
	{
	  char * fallback[3] = {NTXT ("insts,,cycles,,l3m"), NTXT ("insts,,cycles"), NTXT ("insts")};
	  for (int ff = 0; ff < 3; ff++)
	    {
	      int rc = hwc_lookup (kk, 0, fallback[ff], NULL, 0, NULL, NULL);
	      if (rc > 0)
		{
		  default_exp = strdup (fallback[ff]);
		  break;
		}
	    }
	}
      cpcx_default_hwcs[kk] = default_exp;
      cpcx_orig_default_hwcs[kk] = default_exp;
    }

setup_cpc_wrapup:
  if (rc)
    {
      cpcx_npics = 0;
      /*
	      ptr_list_free(&tmp_raw); // free stuff... YXXX
	      ptr_list_free(&unfiltered_attrs);
       */
    }
  return;
}

static void
setup_cpcx ()
{
  if (initialized)
    return;
  setup_cpc_general (0); // set up and include a hwc test run
}

static void
setup_cpc_skip_hwctest ()
{
  if (initialized)
    return;
  setup_cpc_general (1); // set up but skip hwc test run
}

static int
try_a_counter (int forKernel)
{
  if (!VALID_FOR_KERNEL (forKernel))
    return -1;
  int rc = -1;
  const Hwcentry * testevent;
  if (cpcx_std[forKernel] == NULL)
    {
      Tprintf (0, "hwctable: WARNING: cpcx_std not initialized");
      return 0; /* consider this an automatic PASS */
    }
  /* look for a valid table entry, only try valid_cpu_tables[0] */
  {
    testevent = cpcx_std[forKernel][0];
    if (!testevent || !testevent->name)
      {
	Tprintf (0, "hwctable: WARNING: no test metric"
		 " available to verify counters\n");
	return 0; /* consider this an automatic PASS */
      }
    if (REG_LIST_IS_EMPTY (testevent->reg_list))
      return 0; // weird
  }
  Hwcentry tmp_testevent;
  tmp_testevent = *testevent; /* shallow copy */
  if (tmp_testevent.int_name == NULL)
    {
      /* counter is defined in 'hidden' section of table, supply int_name */
      tmp_testevent.int_name = strdup (tmp_testevent.name);
    }
  Hwcentry * test_array[1] = {&tmp_testevent};
  rc = hwcfuncs_assign_regnos (test_array, 1); /* may modify test_array */
  if (rc)
    return rc;
  rc = test_hwcs ((const Hwcentry**) test_array, 1);
  if (rc == HWCFUNCS_ERROR_UNAVAIL)
    {
      // consider this a pass (allow HWC table to be printed)
      Tprintf (0, "hwctable: WARNING: "
	       "cpc_bind_event() shows counters busy; allow to continue\n");
      return 0;
    }
  else if (rc)
    {
      // failed to start for some other reason
      Tprintf (0, "hwctable: WARNING: "
	       "test of counter '%s' failed\n",
	       testevent->name);
      return rc;
    }
  return 0;
}

void
hwc_update_val (Hwcentry *hwc)
{
  if (hwc->ref_val == 0)
    hwc->ref_val = hwc->val; // save original reference
  int64_t newVal;
  hrtime_t min_time_nsec = hwc->min_time;
  if (min_time_nsec == HWCTIME_TBD)
    min_time_nsec = hwc->min_time_default;
  switch (min_time_nsec)
    {
    case 0: // disable time-based intervals
      // do not modify val
      return;
    case HWCTIME_ON:
    case HWCTIME_TBD:
      newVal = HWC_VAL_ON (hwc->ref_val);
      break;
    case HWCTIME_LO:
      newVal = HWC_VAL_LO (hwc->ref_val);
      break;
    case HWCTIME_HI:
      newVal = HWC_VAL_HI (hwc->ref_val);
      break;
    default:
      newVal = HWC_VAL_CUSTOM (hwc->ref_val, min_time_nsec);
      break;
    }
#define MAX_INT_VAL (2*1000*1000*1000 + 1000100)// yuck, limited to signed int
  if (newVal >= MAX_INT_VAL)
    newVal = MAX_INT_VAL;
  hwc->val = newVal;
}

/* convert value string to value and store result in hwc->val */
/* This function moved here from collctrl.cc */
/*
 * Keep the HWCTIME_* definitions in sync with those in
 * collctrl.cc Coll_Ctrl::add_hwcstring().
 */
static int
set_hwcval (Hwcentry *hwc, hrtime_t global_min_time_nsec, const char *valptr)
{
  hwc->min_time_default = global_min_time_nsec;
  if (hwc->val == 1)
    {
      // An interval of 1 is used for certain types of count data.
      // (er_bit, er_generic, er_rock ...)
      // Hi and Lo do not apply.
      /* use the default */
    }
  else if (valptr == NULL || valptr[0] == 0 || strcmp (valptr, "auto") == 0)
    hwc->min_time = HWCTIME_TBD;
  else if (strcmp (valptr, "on") == 0)
    hwc->min_time = HWCTIME_ON;
  else if (strcmp (valptr, "lo") == 0 || strcmp (valptr, "low") == 0)
    hwc->min_time = HWCTIME_LO;
  else if (strcmp (valptr, "hi") == 0 || strcmp (valptr, "high") == 0
	   || strcmp (valptr, "h") == 0)
    hwc->min_time = HWCTIME_HI;
  else
    {
      /* the remaining string should be a number > 0 */
      char *endchar = NULL;
      long long tmp = strtoll (valptr, &endchar, 0);
      int value = (int) tmp;
      if (*endchar != 0 || tmp <= 0 || value != tmp)
	{
	  // also covers errno == ERANGE
	  Tprintf (0, "hwctable: set_hwcval(): ERROR: "
		   "Invalid counter value %s for counter `%s'\n",
		   valptr, hwc->name);
	  return -1;
	}
      if (tmp > UINT32_MAX / 2)
	{
	  /* Roch B. says that we MUST do this check for er_kernel
	     because some platforms deliver overflow interrupts without
	     identifying which counter overflowed.  The only way to
	     determine which counter overflowed is to have enough
	     margin on 32 bit counters to make sure they don't
	     wrap.
	   */
	  Tprintf (0, "hwctable: set_hwcval(): ERROR: "
		   "Counter value %s exceeds %lu\n",
		   valptr, (unsigned long) UINT32_MAX / 2);
	  return -1;
	}
      /* set the value */
      if (value != 0)
	{
	  if (hwc->ref_val == 0)
	    hwc->ref_val = hwc->val; // save original reference
	  hwc->val = value;
	  hwc->min_time = 0; // turn off auto-adjust
	}
    }
  hwc_update_val (hwc);
  return 0;
}

static char *
canonical_name (const char *counter)
{
  char *nameOnly = NULL;
  char *attrs = NULL;
  char tmpbuf[1024];
  tmpbuf[0] = 0;
  hwcfuncs_parse_ctr (counter, NULL, &nameOnly, &attrs, NULL, NULL);
  snprintf (tmpbuf + strlen (tmpbuf), sizeof (tmpbuf) - strlen (tmpbuf),
	    "%s", nameOnly);
  if (attrs)
    {
      hwcfuncs_attr_t cpc2_attrs[HWCFUNCS_MAX_ATTRS];
      void * attr_mem;
      unsigned nattrs;
      int ii, jj;

      /* extract attributes from counter */
      attr_mem = hwcfuncs_parse_attrs (counter, cpc2_attrs, HWCFUNCS_MAX_ATTRS,
				       &nattrs, NULL);
      if (!attr_mem)
	{
	  snprintf (tmpbuf + strlen (tmpbuf), sizeof (tmpbuf) - strlen (tmpbuf),
		    "~UNKNOWN");
	  goto canonical_attrs_wrapup;
	}

      /* sort the attributes */
      for (ii = 0; ii < (int) nattrs - 1; ii++)
	{
	  for (jj = ii + 1; jj < nattrs; jj++)
	    {
	      int cmp = strcmp (cpc2_attrs[ii].ca_name,
				cpc2_attrs[jj].ca_name);
	      if (cmp > 0)
		{
		  hwcfuncs_attr_t tmp = cpc2_attrs[jj];
		  cpc2_attrs[jj] = cpc2_attrs[ii];
		  cpc2_attrs[ii] = tmp;
		}
	    }
	}

      /* print attributes in canonical format */
      for (ii = 0; ii < nattrs; ii++)
	snprintf (tmpbuf + strlen (tmpbuf), sizeof (tmpbuf) - strlen (tmpbuf),
		  "~%s=0x%llx", cpc2_attrs[ii].ca_name, (long long) cpc2_attrs[ii].ca_val);
      free (attr_mem);
    }
canonical_attrs_wrapup:
  free (nameOnly);
  free (attrs);
  return strdup (tmpbuf);
}

/* process counter and value strings - put results in <*pret_ctr> */

/* Print errors to UEbuf for any failure that results in nonzero return */
static int
process_ctr_def (int forKernel, hrtime_t global_min_time_nsec,
		 const char *counter, const char *value, Hwcentry *pret_ctr,
		 char* UWbuf, size_t UWsz, char* UEbuf, size_t UEsz)
{
  int rc = -1;
  char *nameOnly = NULL;
  char *attrs = NULL;
  char *regstr = NULL;
  int plus;
  regno_t regno;
  const Hwcentry *pfound = NULL;
  const char *uname = NULL;
  int disable_backtrack;
  UEbuf[0] = 0;
  UWbuf[0] = 0;
  Tprintf (DBG_LT3, "hwctable: process_ctr_def(): counter=%s value=%s \n",
	   counter, value ? value : "NULL");
  hwcfuncs_parse_ctr (counter, &plus, &nameOnly, &attrs, &regstr, &regno);

  /* search for the counter in the std and raw lists */
  {
    pfound = ptrarray_find ((const Hwcentry**) cpcx_std[forKernel], nameOnly, NULL, 1, regno);
    if (pfound)
      hwcentry_print (DBG_LT1, "hwctable: process_ctr_def: found in stdlist:",
		      pfound);
  }
  if (!pfound)
    {
      pfound = ptrarray_find ((const Hwcentry**) cpcx_hidden[forKernel], nameOnly, NULL, 1, regno);
      if (pfound)
	hwcentry_print (DBG_LT1, "hwctable: process_ctr_def: found in stdlist(hidden):", pfound);
    }
  if (!pfound)
    {
      pfound = ptrarray_find_by_name (cpcx_raw[forKernel], nameOnly); /* (regno match checked later) */
      if (pfound)
	hwcentry_print (DBG_LT1, "hwctable: process_ctr_def: found in rawlist:", pfound);
    }
  if (!pfound)
    {
      pfound = ptrarray_find ((const Hwcentry**) cpcx_std[forKernel], nameOnly, NULL, 1, REGNO_ANY);
      if (pfound)
	hwcentry_print (DBG_LT1, "hwctable: process_ctr_def: found in stdlist but regno didn't match:", pfound);
    }
  if (!pfound)
    {
      pfound = ptrarray_find ((const Hwcentry**) cpcx_hidden[forKernel], nameOnly, NULL, 1, REGNO_ANY);
      if (pfound)
	hwcentry_print (DBG_LT1, "hwctable: process_ctr_def: found in stdlist(hidden) but regno didn't match:", pfound);
    }
  if (!pfound)
    {
      uint64_t val = 0;
      if (is_numeric (nameOnly, &val))
	{
	  Hwcentry *tmp = alloc_shallow_copy (&empty_ctr); // Leaks?
	  if (tmp)
	    {
	      tmp->name = strdup (nameOnly);
	      regno_add (tmp, REGNO_ANY);
	      pfound = tmp;
	    }
	}
      if (pfound)
	hwcentry_print (DBG_LT1, "hwctable: process_ctr_def: counter specified by numeric value:", pfound);
    }
  if (!pfound)
    {
      snprintf (UEbuf + strlen (UEbuf), UEsz - strlen (UEbuf),
		GTXT ("Invalid HW counter name: %s\n"), nameOnly);
      snprintf (UEbuf + strlen (UEbuf), UEsz - strlen (UEbuf),
		GTXT ("Run \"%s -h\" with no other arguments for more information on HW counters on this system.\n"),
		(IS_KERNEL (forKernel) ? "er_kernel" : "collect"));
      goto process_ctr_def_wrapup;
    }

  /* counter found */
  *pret_ctr = *pfound; /* shallow copy */
  pret_ctr->int_name = NULL; /* so free doesn't try to free these pfound's ptrs */
  pret_ctr->name = NULL; /* so free doesn't try to free these pfound's ptrs */

  /* update uname,memop */
  uname = counter;
  disable_backtrack = 0;
  if (plus != 0 || ABST_PLUS_BY_DEFAULT (pret_ctr->memop))
    {
      // attempt to process memoryspace profiling
      int message_printed = 0;
      if (cpcx_cpuver == CPUVER_GENERIC)
	{
	  // accept plus, since we don't know what this CPU is
	  snprintf (UEbuf + strlen (UEbuf), UEsz - strlen (UEbuf),
		    GTXT ("`+' may not be correctly supported on `%s' because processor is not recognized."),
		    cpcx_cciname);
	  pret_ctr->memop = ABST_LDST; // supply a backtracking data type - required for collector
	}
      else if (cpcx_cpuver == CPC_ULTRA1 || cpcx_cpuver == CPC_ULTRA2
	       || cpcx_cpuver == CPC_ULTRA3 || cpcx_cpuver == CPC_ULTRA3_PLUS
	       || cpcx_cpuver == CPC_ULTRA3_I || cpcx_cpuver == CPC_ULTRA4_PLUS
	       || cpcx_cpuver == CPC_ULTRA4 || cpcx_cpuver == CPC_ULTRA_T1
	       || cpcx_cpuver == CPC_ULTRA_T2 || cpcx_cpuver == CPC_ULTRA_T2P
	       || cpcx_cpuver == CPC_ULTRA_T3)
	{
	  if (!ABST_BACKTRACK_ENABLED (pret_ctr->memop))
	    disable_backtrack = 1;
	}
      else if (cpcx_cpuver == CPC_SPARC_T4 || cpcx_cpuver == CPC_SPARC_T5
	       || cpcx_cpuver == CPC_SPARC_T6 || cpcx_cpuver == CPC_SPARC_M4
	       || cpcx_cpuver == CPC_SPARC_M5 || cpcx_cpuver == CPC_SPARC_M6
	       || cpcx_cpuver == CPC_SPARC_M7 || cpcx_cpuver == CPC_SPARC_M8)
	{
	  if (pret_ctr->memop != ABST_EXACT)
	    disable_backtrack = 1;
	}
      else if (cpcx_cpuver == CPC_INTEL_NEHALEM || cpcx_cpuver == CPC_INTEL_WESTMERE
	       || cpcx_cpuver == CPC_INTEL_SANDYBRIDGE
	       || cpcx_cpuver == CPC_INTEL_IVYBRIDGE
	       || cpcx_cpuver == CPC_INTEL_HASWELL
	       || cpcx_cpuver == CPC_INTEL_BROADWELL
	       || cpcx_cpuver == CPC_INTEL_SKYLAKE)
	{
	  if (pret_ctr->memop != ABST_EXACT_PEBS_PLUS1)
	    disable_backtrack = 1;
	  else if (plus < 0)
	    {
	      // disabling memoryspace not supported for
	      // remove specified -
	      uname++;
	      plus = 0;
	      snprintf (UWbuf + strlen (UWbuf), UWsz - strlen (UWbuf),
			GTXT ("Warning: `-' is not supported on `%s' -- memory reference backtracking will remain enabled for this counter\n"),
			nameOnly);
	    }
	}
      else
	{
	  message_printed = 1;
	  snprintf (UWbuf + strlen (UWbuf), UWsz - strlen (UWbuf),
		    GTXT ("Warning: `+' is not supported on `%s' -- memory reference backtracking will not be enabled for `%s'\n"),
		    cpcx_cciname, nameOnly);
	  disable_backtrack = 1;
	}
      if (disable_backtrack)
	{
	  if (plus != 0)
	    uname++;    // remove specified + or -
	  if (!message_printed && plus > 0)
	    snprintf (UWbuf + strlen (UWbuf), UWsz - strlen (UWbuf),
		      GTXT ("Warning: `+' is not supported on `%s' -- memory reference backtracking will not be enabled for this counter\n"),
		      nameOnly);
	}
    }
  else
    disable_backtrack = 1;
  if (disable_backtrack || plus < 0)
    if (pret_ctr->memop != ABST_NOPC)
      pret_ctr->memop = ABST_NONE;
  if (pret_ctr->memop == ABST_NOPC)
    snprintf (UWbuf + strlen (UWbuf), UWsz - strlen (UWbuf),
	      GTXT ("Warning: HW counter `%s' is not program-related -- callstacks will be not be recorded for this counter\n"),
	      uname);

  /* update reg_num */
  if (!regno_is_valid (pfound, regno))
    {
      char buf[1024];
      snprintf (UEbuf + strlen (UEbuf), UEsz - strlen (UEbuf),
		GTXT ("For counter `%s', %s is not a valid register; valid registers: %s\n"),
		nameOnly, regstr ? regstr + 1 : "?",
		get_regnolist (buf, sizeof (buf), pfound->reg_list, 1));
      goto process_ctr_def_wrapup;
    }
  if (pret_ctr->reg_num == REGNO_ANY)
    { /* table's regno is a wildcard */
      if (REG_LIST_EOL (pfound->reg_list[1]))
	{
	  /* valid list only contains one regno, so use it */
	  pret_ctr->reg_num = pfound->reg_list[0];
	}
      else
	pret_ctr->reg_num = regno;  /* use user's selection */
    }

  /* update name and int_name */
  {
    // validate attributes
    if (attrs)
      {
	hwcfuncs_attr_t cpc2_attrs[HWCFUNCS_MAX_ATTRS];
	void * attr_mem;
	unsigned nattrs;
	char *errbuf;
	/* extract attributes from uname */
	attr_mem = hwcfuncs_parse_attrs (uname, cpc2_attrs, HWCFUNCS_MAX_ATTRS,
					 &nattrs, &errbuf);
	if (!attr_mem)
	  {
	    snprintf (UEbuf + strlen (UEbuf), UEsz - strlen (UEbuf),
		      "%s\n", errbuf);
	    free (errbuf);
	    goto process_ctr_def_wrapup;
	  }
	/* make sure all attributes are valid */
	for (unsigned ii = 0; ii < nattrs; ii++)
	  {
	    if (!attr_is_valid (forKernel, cpc2_attrs[ii].ca_name))
	      {
		snprintf (UEbuf + strlen (UEbuf), UEsz - strlen (UEbuf),
			  GTXT ("Invalid attribute specified for counter `%s': %s\n"),
			  nameOnly, cpc2_attrs[ii].ca_name);
		snprintf (UEbuf + strlen (UEbuf), UEsz - strlen (UEbuf),
			  GTXT ("Run \"%s -h\" with no other arguments for more information on HW counters on this system.\n"),
			  (IS_KERNEL (forKernel) ? "er_kernel" : "collect"));
		free (attr_mem);
		goto process_ctr_def_wrapup;
	      }
	    for (unsigned jj = ii + 1; jj < nattrs; jj++)
	      {
		if (strcmp (cpc2_attrs[ii].ca_name,
			    cpc2_attrs[jj].ca_name) == 0)
		  {
		    snprintf (UEbuf + strlen (UEbuf), UEsz - strlen (UEbuf),
			      GTXT ("Duplicate attribute specified for counter `%s': %s\n"),
			      nameOnly, cpc2_attrs[ii].ca_name);
		    free (attr_mem);
		    goto process_ctr_def_wrapup;
		  }
	      }
	  }
	free (attr_mem);
      }
    pret_ctr->name = strdup (uname);

    // assign int_name
    if (pfound->int_name)
      {
	// Counter is one of the following:
	// - aliased (e.g. cycles~system=1),
	// - convenience (e.g. cycles0~system=1),
	if (!attrs) // convert alias to internal name
	  pret_ctr->int_name = strdup (pfound->int_name);
	else
	  {
	    // convert alias to internal name and
	    // append user-supplied attributes
	    size_t sz = strlen (pfound->int_name) + strlen (attrs) + 1;
	    char *tbuf = calloc (sz, 1);
	    if (tbuf)
	      snprintf (tbuf, sz, "%s%s", pfound->int_name, attrs);
	    pret_ctr->int_name = tbuf;
	  }
      }
    else
      pret_ctr->int_name = strdup (uname);  // user-supplied name
  }

  /* update val */
  if (set_hwcval (pret_ctr, global_min_time_nsec, value))
    {
      snprintf (UEbuf + strlen (UEbuf), UEsz - strlen (UEbuf),
		GTXT ("Invalid interval for HW counter `%s': %s\n"),
		nameOnly, value);
      goto process_ctr_def_wrapup;
    }
  hwcentry_print (DBG_LT2, "hwctable: process_ctr_def:", pret_ctr);
  rc = 0;

process_ctr_def_wrapup:
  free (regstr);
  free (attrs);
  free (nameOnly);
  return rc;
}

/*---------------------------------------------------------------------------*/

/* external interfaces, see hwcentry.h for descriptions. */

extern int
hwc_lookup (int forKernel, hrtime_t global_min_time_nsec, const char *instring,
	    Hwcentry *caller_entries[], unsigned maxctrs, char **emsg, char **wmsg)
{
  unsigned ii;
  char *instr_copy = NULL, *ss = NULL;
  unsigned numctrs = 0;
  int rc = 0;
  char *tokenptr[MAX_PICS * 2];
  unsigned numtokens = 0;
  char UEbuf[1024 * 5]; /* error message buffer; strdup of it is passed back to user  */
  char UWbuf[1024 * 5]; /* warning message buffer; strdup of it is passed back to user  */
  if (emsg)
    *emsg = NULL;
  if (wmsg)
    *wmsg = NULL;
  UEbuf[0] = 0;
  UWbuf[0] = 0;

  // supply temporary result buffers as needed
  Hwcentry tmp_entry_table[MAX_PICS];
  Hwcentry * tmp_entries[MAX_PICS];
  Hwcentry **entries;
  if (caller_entries)
    entries = caller_entries;
  else
    {
      // user doesn't care about results; provide temporary storage for results
      for (ii = 0; ii < MAX_PICS; ii++)
	tmp_entries[ii] = &tmp_entry_table[ii];
      entries = tmp_entries;
      maxctrs = MAX_PICS;
    }
  Tprintf (DBG_LT1, "hwctable: hwc_lookup(%s)\n",
	   instring ? instring : "NULL");

  /* clear <entries> first - prevent seg faults in hwc_lookup_wrapup */
  for (ii = 0; ii < maxctrs; ii++)
    *entries[ii] = empty_ctr;
  if (!instring)
    {
      snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		GTXT ("No HW counters were specified."));
      rc = -1;
      goto hwc_lookup_wrapup;
    }

  /* make sure tables are initialized */
  setup_cpc_skip_hwctest ();
  if (cpcx_npics == 0)
    {
      if (cpcx_cpuver < 0)
	{
	  char buf[1024];
	  *buf = 0;
	  char *pch = hwcfuncs_errmsg_get (buf, sizeof (buf), 0); /* get first err msg, disable capture */
	  if (*pch)
	    snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		      GTXT ("HW counter profiling is not supported on this system: %s%s"),
		      pch, pch[strlen (pch) - 1] == '\n' ? "" : "\n");
	  else
	    snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		      GTXT ("HW counter profiling is not supported on this system\n"));
	}
      else
	snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		  GTXT ("HW counter profiling is not supported on '%s'\n"),
		  cpcx_cciname);
      rc = -1;
      goto hwc_lookup_wrapup;
    }
  ss = instr_copy = strdup (instring);
  while (*ss != 0 && (*ss == ' ' || *ss == '\t'))
    ss++;
  tokenptr[numtokens++] = ss;
  do
    {
      /* find end of previous token, replace w/ NULL, skip whitespace, set <tokenptr>, repeat */
      for (; *ss; ss++)
	{
	  if (*ss == ',' || *ss == ' ' || *ss == '\t')
	    {
	      /* end of previous token found */
	      *ss = 0; /* terminate the previous token */
	      ss++;
	      while (*ss != 0 && (*ss == ' ' || *ss == '\t'))
		ss++;
	      if (*ss)
		tokenptr[numtokens++] = ss;
	      break; // from for loop
	    }
	}
    }
  while (*ss && numtokens < (MAX_PICS * 2));

  if (*ss)
    {
      snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		GTXT ("The number of HW counters specified exceeds internal resources\n"));
      snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		GTXT ("Run \"%s -h\" with no other arguments for more information on HW counters on this system.\n"),
		(IS_KERNEL (forKernel) ? "er_kernel" : "collect"));
      rc = -1;
      goto hwc_lookup_wrapup;
    }
  Tprintf (DBG_LT3, "hwctable: hwc_lookup(): numtokens=%d\n", numtokens);

  /* look up individual counters */
  {
    int fail = 0;
    for (ii = 0; ii < numtokens && numctrs < maxctrs; ii += 2)
      {
	const char *counter;
	const char *value;
	Hwcentry *pret_ctr = entries[numctrs];

	/* assign the tokens to ctrnames, timeoutValues. */
	counter = tokenptr[ii];
	if (ii + 1 < numtokens)
	  value = tokenptr[ii + 1];
	else
	  value = 0;
	if (process_ctr_def (forKernel, global_min_time_nsec, counter, value, pret_ctr,
			     UWbuf + strlen (UWbuf),
			     sizeof (UWbuf) - strlen (UWbuf),
			     UEbuf + strlen (UEbuf),
			     sizeof (UEbuf) - strlen (UEbuf)))
	  {
	    /* could choose to set fail=1 and continue here,
	       but errmsgs would be aggregated (messy) */
	    rc = -1;
	    goto hwc_lookup_wrapup;
	  }
	numctrs++;
      }
    if (fail)
      {
	rc = -1;
	goto hwc_lookup_wrapup;
      }
  }

  if (!numctrs)
    {
      snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		GTXT ("No HW counters were specified.\n"));
      rc = -1;
      goto hwc_lookup_wrapup;
    }
  if (numctrs > cpcx_max_concurrent[forKernel])
    {
      snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		GTXT ("The HW counter configuration could not be loaded: More than %d counters were specified\n"), cpcx_max_concurrent[forKernel]);
      snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		GTXT ("Run \"%s -h\" with no other arguments for more information on HW counters on this system.\n"),
		(IS_KERNEL (forKernel) ? "er_kernel" : "collect"));
      rc = -1;
      goto hwc_lookup_wrapup;
    }

hwc_lookup_wrapup:
  free (instr_copy);
  if (wmsg && strlen (UWbuf))
    *wmsg = strdup (UWbuf);
  if (emsg && strlen (UEbuf))
    *emsg = strdup (UEbuf);
  if (rc == 0)
    rc = numctrs;
  return rc;
}

extern char *
hwc_validate_ctrs (int forKernel, Hwcentry *entries[], unsigned numctrs)
{
  char UEbuf[1024 * 5];
  UEbuf[0] = 0;

  /* search for obvious duplicates*/
  unsigned ii;
  for (ii = 0; ii < numctrs; ii++)
    {
      regno_t reg_a = entries[ii]->reg_num;
      if (reg_a != REGNO_ANY)
	{
	  unsigned jj;
	  for (jj = ii + 1; jj < numctrs; jj++)
	    {
	      int reg_b = entries[jj]->reg_num;
	      if (reg_a == reg_b)
		{
		  snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
			    GTXT ("Only one HW counter is allowed per register.  The following counters use register %d: \n"),
			    reg_a);
		  for (jj = 0; jj < numctrs; jj++)
		    {
		      char buf[256];
		      int reg_b = entries[jj]->reg_num;
		      if (reg_a == reg_b)
			snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
				  GTXT ("  %d. %s\n"), jj + 1,
				  hwc_hwcentry_specd_string (buf, sizeof (buf),
							     entries[jj]));
		    }
		  return strdup (UEbuf);
		}
	    }
	}
    }

  /* test counters */
  hwcfuncs_errmsg_get (NULL, 0, 1); /* enable errmsg capture */
  int hwc_rc = hwcfuncs_assign_regnos (entries, numctrs);
  if (!hwc_rc)
    hwc_rc = test_hwcs ((const Hwcentry**) entries, numctrs);
  if (hwc_rc)
    {
      if (cpcx_cpuver == CPC_PENTIUM_4_HT || cpcx_cpuver == CPC_PENTIUM_4)
	{
	  snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		    GTXT ("HW counter profiling is disabled unless only one logical CPU per HyperThreaded processor is online (see psradm)\n"));
	  return strdup (UEbuf);
	}
      char buf[1024];
      *buf = 0;
      char * pch = hwcfuncs_errmsg_get (buf, sizeof (buf), 0); /* get first err msg, disable capture */
      if (*pch)
	snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		  GTXT ("The HW counter configuration could not be loaded: %s%s"),
		  pch, pch[strlen (pch) - 1] == '\n' ? "" : "\n");
      else
	snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		  GTXT ("The HW counter configuration could not be loaded\n"));
      snprintf (UEbuf + strlen (UEbuf), sizeof (UEbuf) - strlen (UEbuf),
		GTXT ("Run \"%s -h\" with no other arguments for more information on HW counters on this system.\n"),
		(IS_KERNEL (forKernel) ? "er_kernel" : "collect"));
      return strdup (UEbuf);
    }
  return NULL;
}

extern Hwcentry *
hwc_post_lookup (Hwcentry * pret_ctr, char *counter, char * int_name, int cpuver)
{
  const Hwcentry *pfound;
  regno_t regno;
  char *nameOnly = NULL;
  char *attrs = NULL;

  /* fields in pret_ctr (name and int_name) should already be free */
  hwcfuncs_parse_ctr (counter, NULL, &nameOnly, &attrs, NULL, &regno);

  /* look for it in the canonical list */
  pfound = static_table_find (stdlist_get_table (cpuver),
			      nameOnly, int_name, 0, REGNO_ANY);
  if (!pfound)  /* try the generic list */
    pfound = static_table_find (papi_generic_list,
				nameOnly, int_name, 0, REGNO_ANY);
  if (pfound)
    {
      /* in standard list */
      *pret_ctr = *pfound; /* shallow copy */
      if (pret_ctr->int_name)
	{
	  // aliased counter
	  pret_ctr->int_name = strdup (pret_ctr->int_name);
	  if (pret_ctr->short_desc == NULL)
	    {
	      // look for short_desc of corresponding raw counter
	      const Hwcentry *praw = static_table_find (stdlist_get_table (cpuver),
							pret_ctr->int_name, NULL, 0, REGNO_ANY);
	      if (praw && praw->short_desc)
		pret_ctr->short_desc = strdup (praw->short_desc);
	    }
	}
      else
	pret_ctr->int_name = strdup (counter);
      if (pret_ctr->reg_num == REGNO_ANY)
	pret_ctr->reg_num = regno;  /* table's regno is a wildcard */
    }
  else
    {
      /* not a standard counter */
      *pret_ctr = empty_ctr;
      pret_ctr->int_name = strdup (counter);
      pret_ctr->reg_num = regno;
    }

  /* update the name */
  if (attrs)
    {
      pret_ctr->name = canonical_name (counter);
      if (pret_ctr->metric)
	{
	  // metric text is supplied from a table. (User supplied HWC alias)
	  // Append user-supplied attributes to metric name:
	  size_t len = strlen (pret_ctr->metric) + strlen (attrs) + 4;
	  char *pch = calloc (len, 1);
	  if (pch)
	    snprintf (pch, len, "%s (%s)", pret_ctr->metric, attrs);
	  pret_ctr->metric = pch; // leaks
	}
    }
  else
    pret_ctr->name = strdup (nameOnly);

  if (pfound)
    hwcentry_print (DBG_LT2, "hwctable: hwc_post_lookup: found: ", pret_ctr);
  else
    hwcentry_print (DBG_LT2, "hwctable: hwc_post_lookup: default: ", pret_ctr);
  free (attrs);
  free (nameOnly);
  return pret_ctr;
}

static const char *
hwc_on_lo_hi (const Hwcentry *pctr)
{
  char* rate;
  {
    switch (pctr->min_time)
      {
      case (HWCTIME_LO):
	rate = NTXT ("lo");
	break;
      case (HWCTIME_ON):
	rate = NTXT ("on");
	break;
      case (HWCTIME_HI):
	rate = NTXT ("hi");
	break;
      case (0):
	rate = NULL; // null => use interval count
	break;
      default:
      case (HWCTIME_TBD):
	rate = NTXT ("on");
	break;
      }
  }
  return rate; //strdup( rate );
}

extern char *
hwc_rate_string (const Hwcentry *pctr, int force_numeric)
{
  const char * rateString = hwc_on_lo_hi (pctr);
  char buf[128];
  if (!rateString || force_numeric)
    {
      snprintf (buf, sizeof (buf), NTXT ("%d"), pctr->val);
      rateString = buf;
    }
  return strdup (rateString);
}

static char metricbuf[2048];

extern char *
hwc_i18n_metric (const Hwcentry *pctr)
{
  if (pctr->metric != NULL)
    snprintf (metricbuf, sizeof (metricbuf), NTXT ("%s"), PTXT (pctr->metric));
  else if (pctr->name != NULL)
    snprintf (metricbuf, sizeof (metricbuf), GTXT ("%s Events"), pctr->name);
  else if (pctr->int_name != NULL)
    snprintf (metricbuf, sizeof (metricbuf), GTXT ("%s Events"), pctr->int_name);
  else
    snprintf (metricbuf, sizeof (metricbuf), GTXT ("Undefined Events"));
  return metricbuf;
}

/* return cpu version, should only be called when about to generate an experiment,
   not when reading back an experiment */
#if 0 /* called by ... */
. / perfan / collect / src / collect.cc : start : 245 : cpuver = hwc_get_cpc_cpuver ();
. / ccr_components / Collector_Interface / collctrl.cc : constructor : 202 : cpcx_cpuver = hwc_get_cpc_cpuver ();
. / perfan / dbe / src / Dbe.cc : 3041 : JApplication::cpuver = hwc_get_cpc_cpuver ();
. / perfan / dbe / src / Dbe.cc : 3164 : JApplication::cpuver = hwc_get_cpc_cpuver ();

note:
cpc_getcpuver () : only papi, ostest, this and hwprofile.c call it
#endif
int
hwc_get_cpc_cpuver ()
{
  setup_cpcx ();
  return cpcx_cpuver;
}

extern char*
hwc_get_cpuname (char *buf, size_t buflen)
{
  setup_cpcx ();
  if (!buf || !buflen)
    return buf;
  buf[0] = 0;
  if (cpcx_cciname)
    {
      strncpy (buf, cpcx_cciname, buflen - 1);
      buf[buflen - 1] = 0;
    }
  return buf;
}

extern char*
hwc_get_docref (char *buf, size_t buflen)
{
  setup_cpcx ();
  if (!buf || !buflen)
    return buf;
  buf[0] = 0;
  if (cpcx_docref)
    {
      strncpy (buf, cpcx_docref, buflen - 1);
      buf[buflen - 1] = 0;
    }
  return buf;
}

//TBR:

extern char*
hwc_get_default_cntrs ()
{
  setup_cpcx ();
  if (cpcx_default_hwcs[0] != NULL)
    return strdup (cpcx_default_hwcs[0]); // TBR deprecate this
  return NULL;
}

extern char*
hwc_get_default_cntrs2 (int forKernel, int style)
{
  setup_cpcx ();
  if (!VALID_FOR_KERNEL (forKernel))
    return NULL;
  char *cpcx_default = cpcx_default_hwcs[forKernel];
  if (cpcx_default == NULL || cpcx_npics == 0)
    return NULL;
  if (style == 1)
    return strdup (cpcx_default);

  // style == 2
  // we will replace "," delimiters with " -h " (an extra 3 chars per HWC)
  char *s = (char *) malloc (strlen (cpcx_default) + 3 * cpcx_npics);
  if (s == NULL) return s;
  char *p = s;
  char *q = cpcx_default;
  int i;
  for (i = 0; i < cpcx_npics; i++)
    {
      int qlen = strlen (q);
      if (qlen == 0)
	{
	  p[0] = '\0';
	  break;
	}
      // add " -h " if not the first HWC
      if (i != 0)
	{
	  p[0] = ' ';
	  p[1] = '-';
	  p[2] = 'h';
	  p[3] = ' ';
	  p += 4;
	}

      // find second comma
      char *r = strchr (q, ',');
      if (r)
	r = strchr (r + 1, ',');

      // we didn't find one, so the rest of the string is the last HWC
      if (r == NULL)
	{
	  // EUGENE could check i==cpcx_npicx-1, but what if it isn't???
	  strcpy (p, q);
	  if (p[qlen - 1] == ',')
	    qlen--;
	  p[qlen] = '\0';
	  break;
	}

      // copy the HWC, trim trailing comma, add null char
      qlen = r - q - 1;
      strcpy (p, q);
      if (p[qlen - 1] == ',')
	qlen--;
      p += qlen;
      p[0] = '\0';
      q = r + 1;
    }
  return s;
}

extern char*
hwc_get_orig_default_cntrs (int forKernel)
{
  setup_cpcx ();
  if (!VALID_FOR_KERNEL (forKernel))
    return NULL;
  if (cpcx_orig_default_hwcs[forKernel] != NULL)
    return strdup (cpcx_orig_default_hwcs[forKernel]);
  return NULL;
}

extern const char *
hwc_memop_string (ABST_type memop)
{
  const char * s;
  switch (memop)
    {
    case ABST_NONE:
      s = "";
      break;
    case ABST_LOAD:
      s = GTXT ("load ");
      break;
    case ABST_STORE:
      s = GTXT ("store ");
      break;
    case ABST_LDST:
    case ABST_US_DTLBM:
    case ABST_LDST_SPARC64:
      s = GTXT ("load-store ");
      break;
    case ABST_EXACT_PEBS_PLUS1:
    case ABST_EXACT:
      s = GTXT ("memoryspace ");
      break;
    case ABST_COUNT:
      s = GTXT ("count ");
      break;
    case ABST_NOPC:
      s = GTXT ("not-program-related ");
      break;
    default:
      s = ""; // was "ABST_UNK", but that's meaningless to users
      break;
    }
  return s;
}

static const char *
timecvt_string (int timecvt)
{
  if (timecvt > 0)
    return GTXT ("CPU-cycles");
  if (timecvt < 0)
    return GTXT ("ref-cycles");
  return GTXT ("events");
}

int show_regs = 0;  // The register setting is available on Solaris only

/*
 * print the specified strings in aligned columns
 */
static void
format_columns (char *buf, int bufsiz, char *s1, char *s2, const char *s3,
		const char *s4, char *s5, const char *s6)
{
  // NULL strings are blanks
  char *blank = NTXT ("");
  if (s2 == NULL)
    s2 = blank;
  if (s3 == NULL)
    s3 = blank;
  if (s6 == NULL)
    s6 = blank;

  // get the lengths and target widths
  // (s6 can be as wide as it likes)
  int l1 = strlen (s1), n1 = 10, l2 = strlen (s2), n2 = 13;
  int l3 = strlen (s3), n3 = 20, l4 = strlen (s4), n4 = 10, n5;
  char divide = ' ';

  // adjust widths, stealing from one column to help a neighbor
  // There's a ragged boundary between s2 and s3.
  // So push this boundary to the right.
  n2 += n3 - l3;
  n3 -= n3 - l3;

  // If s3 is empty, push the boundary over to s4.
  if (l3 == 0)
    {
      n2 += n4 - l4;
      n4 -= n4 - l4;
    }

  // If there's enough room to fit s1 and s2, do so.
  if (n1 + n2 >= l1 + l2)
    {
      if (n1 < l1)
	{
	  n2 -= l1 - n1;
	  n1 += l1 - n1;
	}
      if (n2 < l2)
	{
	  n1 -= l2 - n2;
	  n2 += l2 - n2;
	}
    }
  else
    {
      // not enough room, so we need to divide the line
      n3 += 4 // 4-blank margin
	      + n1 // 1st column
	      + 1 // space between 1st and 2nd columns
	      + n2 // 2nd column
	      + 1; // space between 2nd and 3th columns
      divide = '\n';

      // make 1st column large enough
      if (n1 < l1)
	n1 = l1;

      // width of 2nd column no longer matters since we divided the line
      n2 = 0;
    }

  if (show_regs)
    {
      // fifth column should be wide enough for regnolist
      //     see function get_regnolist()
      if (cpcx_npics < 10)
	n5 = cpcx_npics; // one char per regno
      else
	n5 = 16 + 3 * (cpcx_npics - 9); // spaces between regnos and some regnos are 2-char wide
      // ... and be wide enough for header "regs"
      if (n5 < 4)
	n5 = 4;

      // print to buffer
      // (don't need a space before s4 since historical precedent to have a trailing space in s3)
      snprintf (buf, bufsiz, "%-*s %-*s%c%*s%*s %-*s %s",
		n1, s1, n2, s2, divide, n3, s3, n4, s4, n5, s5, s6);
    }
  else
    snprintf (buf, bufsiz, "%-*s %-*s%c%*s%*s %s",
	      n1, s1, n2, s2, divide, n3, s3, n4, s4, s6);
  for (int i = strlen (buf); i > 0; i--)
    if (buf[i] == ' ' || buf[i] == '\t')
      buf[i] = 0;
    else
      break;
}

/* routine to return HW counter string formatted and i18n'd */
static char *
hwc_hwcentry_string_internal (char *buf, size_t buflen, const Hwcentry *ctr,
			      int show_short_desc)
{
  char regnolist[256];
  if (!buf || !buflen)
    return buf;
  if (ctr == NULL)
    {
      snprintf (buf, buflen, GTXT ("HW counter not available"));
      return buf;
    }
  char *desc = NULL;
  if (show_short_desc)
    desc = ctr->short_desc;
  if (desc == NULL)
    desc = ctr->metric ? hwc_i18n_metric (ctr) : NULL;
  format_columns (buf, buflen, ctr->name, ctr->int_name,
		  hwc_memop_string (ctr->memop), timecvt_string (ctr->timecvt),
		  get_regnolist (regnolist, sizeof (regnolist), ctr->reg_list, 2),
		  desc);
  return buf;
}

/* routine to return HW counter string formatted and i18n'd */
extern char *
hwc_hwcentry_string (char *buf, size_t buflen, const Hwcentry *ctr)
{
  return hwc_hwcentry_string_internal (buf, buflen, ctr, 0);
}

/* routine to return HW counter string formatted and i18n'd */
extern char *
hwc_hwcentry_specd_string (char *buf, size_t buflen, const Hwcentry *ctr)
{
  const char *memop, *timecvt;
  char descstr[1024];
  if (!buf || !buflen)
    return buf;
  if (ctr == NULL)
    {
      snprintf (buf, buflen, GTXT ("HW counter not available"));
      return buf;
    }
  timecvt = timecvt_string (ctr->timecvt);
  if (ctr->memop)
    memop = hwc_memop_string (ctr->memop);
  else
    memop = "";
  if (ctr->metric != NULL)  /* a standard counter for a specific register */
    snprintf (descstr, sizeof (descstr), " (`%s'; %s%s)",
	      hwc_i18n_metric (ctr), memop, timecvt);
  else  /* raw counter */
    snprintf (descstr, sizeof (descstr), " (%s%s)", memop, timecvt);

  char *rateString = hwc_rate_string (ctr, 1);
  snprintf (buf, buflen, "%s,%s%s", ctr->name,
	    rateString ? rateString : "", descstr);
  free (rateString);
  return buf;
}

unsigned
hwc_get_max_regs ()
{
  setup_cpcx ();
  return cpcx_npics;
}

unsigned
hwc_get_max_concurrent (int forKernel)
{
  setup_cpcx ();
  if (!VALID_FOR_KERNEL (forKernel))
    return 0;
  return cpcx_max_concurrent[forKernel];
}

char**
hwc_get_attrs (int forKernel)
{
  setup_cpcx ();
  if (!VALID_FOR_KERNEL (forKernel))
    return NULL;
  return cpcx_attrs[forKernel];
}

Hwcentry **
hwc_get_std_ctrs (int forKernel)
{
  setup_cpcx ();
  if (!VALID_FOR_KERNEL (forKernel))
    return NULL;
  return cpcx_std[forKernel];
}

Hwcentry **
hwc_get_raw_ctrs (int forKernel)
{
  setup_cpcx ();
  if (!VALID_FOR_KERNEL (forKernel))
    return NULL;
  return cpcx_raw[forKernel];
}

/* Call an action function for each attribute supported */
unsigned
hwc_scan_attrs (void (*action)(const char *attr, const char *desc))
{
  setup_cpcx ();
  int cnt = 0;
  for (int ii = 0; cpcx_attrs[0] && cpcx_attrs[0][ii]; ii++, cnt++)
    {
      if (action)
	action (cpcx_attrs[0][ii], NULL);
    }
  if (!cnt && action)
    action (NULL, NULL);
  return cnt;
}

unsigned
hwc_scan_std_ctrs (void (*action)(const Hwcentry *))
{
  setup_cpcx ();
  Tprintf (DBG_LT1, "hwctable: hwc_scan_standard_ctrs()...\n");
  int cnt = 0;
  for (int ii = 0; cpcx_std[0] && cpcx_std[0][ii]; ii++, cnt++)
    if (action)
      action (cpcx_std[0][ii]);
  if (!cnt && action)
    action (NULL);
  return cnt;
}

/* Call an action function for each counter supported */
/* action is called with NULL when all counters have been seen */
unsigned
hwc_scan_raw_ctrs (void (*action)(const Hwcentry *))
{
  setup_cpcx ();
  Tprintf (DBG_LT1, "hwctable: hwc_scan_raw_ctrs()...\n");
  int cnt = 0;
  for (int ii = 0; cpcx_raw[0] && cpcx_raw[0][ii]; ii++, cnt++)
    if (action)
      action (cpcx_raw[0][ii]);
  if (!cnt && action)
    action (NULL);
  return cnt;
}

static void
hwc_usage_raw_overview_sparc (FILE *f_usage, int cpuver)
{
  /* All these cpuver's use cputabs[]==sparc_t5_m6 anyhow. */
  if ((cpuver == CPC_SPARC_M5) || (cpuver == CPC_SPARC_M6)
      || (cpuver == CPC_SPARC_T5) || (cpuver == CPC_SPARC_T6))
    cpuver = CPC_SPARC_M4; // M4 was renamed to M5

  /* While there are small differences between
   *     cputabs[]== sparc_t4
   *     cputabs[]== sparc_t5_m6
   * they are in HWCs we don't discuss in the overview anyhow.
   * So just lump them in with T4.
   */
  if (cpuver == CPC_SPARC_M4)
    cpuver = CPC_SPARC_T4;

  /* Check for the cases we support. */
  if (cpuver != CPC_SPARC_T4 && cpuver != CPC_SPARC_M7 && cpuver != CPC_SPARC_M8)
    return;
  fprintf (f_usage, GTXT ("    While the above aliases represent the most useful hardware counters\n"
			  "    for this processor, a full list of raw (unaliased) counter names appears\n"
			  "    below.  First is an overview of some of these names.\n\n"));
  fprintf (f_usage, GTXT ("        == Cycles.\n"
			  "        Count active cycles with\n"
			  "            Cycles_user\n"
			  "        Set attributes to choose user, system, and/or hyperprivileged cycles.\n\n"));
  fprintf (f_usage, GTXT ("        == Instructions.\n"
			  "        Count instructions when they are committed with:\n"));
  fprintf (f_usage, NTXT ("            Instr_all\n"));
  if (cpuver != CPC_SPARC_M8)
    fprintf (f_usage, GTXT ("        It is the total of these counters:\n"));
  else
    fprintf (f_usage, GTXT ("        Some subsets of instructions can be counted separately:\n"));
  fprintf (f_usage, NTXT ("            Branches               %s\n"), GTXT ("branches"));
  fprintf (f_usage, NTXT ("            Instr_FGU_crypto       %s\n"), GTXT ("Floating Point and Graphics Unit"));
  fprintf (f_usage, NTXT ("            Instr_ld               %s\n"), GTXT ("loads"));
  fprintf (f_usage, NTXT ("            Instr_st               %s\n"), GTXT ("stores"));
  fprintf (f_usage, NTXT ("            %-19s    %s\n"),
	   cpuver == CPC_SPARC_M7 ? NTXT ("Instr_SPR_ring_ops")
	   : NTXT ("SPR_ring_ops"),
	   GTXT ("internal use of SPR ring"));
  fprintf (f_usage, NTXT ("            Instr_other            %s\n"), GTXT ("basic arithmetic and logical instructions"));
  if (cpuver != CPC_SPARC_M8)
    fprintf (f_usage, GTXT ("        Some subsets of these instructions can be counted separately:\n"));
  fprintf (f_usage, NTXT ("            Br_taken               %s\n"), GTXT ("Branches that are taken"));
  fprintf (f_usage, NTXT ("            %-19s    %s\n"),
	   cpuver == CPC_SPARC_M7 ? NTXT ("Instr_block_ld_st")
	   : NTXT ("Block_ld_st"),
	   GTXT ("block load/store"));
  fprintf (f_usage, NTXT ("            %-19s    %s\n"),
	   cpuver == CPC_SPARC_M7 ? NTXT ("Instr_atomic")
	   : NTXT ("Atomics"),
	   GTXT ("atomic instructions"));
  fprintf (f_usage, NTXT ("            %-19s    %s\n"),
	   cpuver == CPC_SPARC_M7 ? NTXT ("Instr_SW_prefetch")
	   : NTXT ("SW_prefetch"),
	   GTXT ("prefetches"));
  fprintf (f_usage, NTXT ("            %-19s    %s\n"),
	   cpuver == CPC_SPARC_M7 ? NTXT ("Instr_SW_count")
	   : NTXT ("Sw_count_intr"),
	   GTXT ("SW Count instructions (counts special no-op assembler instructions)"));
  fprintf (f_usage, NTXT ("\n"));

#ifdef TMPLEN
  compilation error : we're trying to use a macro that's already defined
#endif
#define TMPLEN 32
	  char s0[TMPLEN], s1[TMPLEN], s2[TMPLEN], s3[TMPLEN];
  if (cpuver == CPC_SPARC_M7)
    {
      snprintf (s0, TMPLEN, "Commit_0_cyc");
      snprintf (s1, TMPLEN, "Commit_1_cyc");
      snprintf (s2, TMPLEN, "Commit_2_cyc");
      snprintf (s3, TMPLEN, "Commit_1_or_2_cyc");
    }
  else
    {
      snprintf (s0, TMPLEN, "Commit_0");
      snprintf (s1, TMPLEN, "Commit_1");
      snprintf (s2, TMPLEN, "Commit_2");
      snprintf (s3, TMPLEN, "Commit_1_or_2");
    }
#undef TMPLEN
  fprintf (f_usage, GTXT ("        == Commit.\n"
			  "        Instructions may be launched speculatively, executed out of order, etc.\n"));
  if (cpuver != CPC_SPARC_M8)
    {
      fprintf (f_usage, GTXT ("        We can count the number of cycles during which 0, 1, or 2 instructions are\n"
			      "        actually completed and their results committed:\n"));
      fprintf (f_usage, GTXT ("            %s\n"
			      "            %s\n"
			      "            %s\n"
			      "            %s\n"
			      "        %s is a useful way of identifying parts of your application with\n"
			      "        high-latency instructions.\n\n"),
	       s0, s1, s2, s3, s0);
    }
  else
    {
      fprintf (f_usage, GTXT ("        We can count the number of cycles during which no instructions were\n"
			      "        able to commit results using:\n"));
      fprintf (f_usage, GTXT ("            %s\n"
			      "        %s is a useful way of identifying parts of your application with\n"
			      "        high-latency instructions.\n\n"),
	       s0, s0);
    }

  fprintf (f_usage, GTXT ("        == Cache/memory hierarchy.\n"));
  if (cpuver == CPC_SPARC_M7)
    {
      fprintf (f_usage, GTXT ("        In the cache hierarchy:\n"
			      "         * Each socket has memory and multiple SPARC core clusters (scc).\n"
			      "         * Each scc has an L3 cache and multiple L2 and L1 caches.\n"));
      fprintf (f_usage, GTXT ("        Loads can be counted by where they hit on socket:\n"));
      fprintf (f_usage, NTXT ("            %-22s %s\n"),
	       NTXT ("DC_hit"), GTXT ("hit own L1 data cache"));
      fprintf (f_usage, NTXT ("            %-22s %s\n"),
	       NTXT ("DC_miss_L2_hit"), GTXT ("hit own L2"));
      fprintf (f_usage, NTXT ("            %-22s %s\n"),
	       NTXT ("DC_miss_L3_hit"), GTXT ("hit own L3"));
      fprintf (f_usage, NTXT ("            %-22s %s\n"),
	       NTXT ("DC_miss_nbr_L2_hit"), GTXT ("hit neighbor L2  (same scc)"));
      fprintf (f_usage, NTXT ("            %-22s %s\n"),
	       NTXT ("DC_miss_nbr_scc_hit"), GTXT ("hit neighbor scc (same socket)"));
      fprintf (f_usage, NTXT ("            %-22s %s\n"),
	       NTXT ("DC_miss_nbr_scc_miss"), GTXT ("miss all caches  (same socket)"));
      fprintf (f_usage, GTXT ("        These loads can also be grouped:\n"));
      fprintf (f_usage, NTXT ("            %-22s %s\n"),
	       NTXT ("DC_miss"), GTXT ("all - DC_hit"));
      fprintf (f_usage, NTXT ("            %-22s %s\n"),
	       NTXT ("DC_miss_L2_miss"), GTXT ("all - DC_hit - DC_miss_L2_hit"));
      fprintf (f_usage, NTXT ("            %-22s %s\n"),
	       NTXT ("DC_miss_L3_miss"), GTXT ("DC_miss_nbr_scc_hit + DC_miss_nbr_scc_miss"));
      fprintf (f_usage, GTXT ("        Loads that miss all caches on this socket can be counted:\n"));
      fprintf (f_usage, NTXT ("            %-22s %s\n"),
	       NTXT ("DC_miss_remote_scc_hit"), GTXT ("hit cache on different socket"));
      fprintf (f_usage, NTXT ("            %-22s %s\n"),
	       NTXT ("DC_miss_local_mem_hit"), GTXT ("hit local memory (same socket)"));
      fprintf (f_usage, NTXT ("            %-22s %s\n"),
	       NTXT ("DC_miss_remote_mem_hit"), GTXT ("hit remote memory (off socket)"));
      fprintf (f_usage, GTXT ("        These events are for speculative loads, launched in anticipation\n"
			      "        of helping performance but whose results might not be committed.\n"));
#if 0 // was: #if defined(linux).  See 22236226 - sparc-Linux: Support basic Memoryspace and Dataspace profiling (capture VADDR)
      /* 21869427 should not look like memoryspace profiling is supported on Linux */
      /* 21869424 desire memoryspace profiling on Linux */
      fprintf (f_usage, GTXT ("        To count only data-cache misses that commit, use:\n"));
      fprintf (f_usage, NTXT ("            DC_miss_commit\n"));
#else
      fprintf (f_usage, GTXT ("        To count only data-cache misses that commit, or for memoryspace profiling,\n"
			      "        use the 'memoryspace' counter:\n"));
      fprintf (f_usage, NTXT ("            DC_miss_commit\n"));
#endif
      fprintf (f_usage, NTXT ("\n"));
    }
  else if (cpuver == CPC_SPARC_M8)
    {
      fprintf (f_usage, GTXT ("        In the cache hierarchy:\n"
			      "         * Each processor has 4 memory controllers and 2 quad core clusters (QCC).\n"
			      "         * Each QCC contains 4 cache processor clusters (CPC).\n"
			      "         * Each CPC contains 4 cores.\n"
			      "         * Each core supports 8 hardware threads.\n"
			      "         * The L3 consists of 2 partitions with 1 QCC per partition.\n"
			      ));
      fprintf (f_usage, GTXT ("        Loads can be counted by where they hit on socket:\n"));
      fprintf (f_usage, NTXT ("            %-22s %s\n"),
	       NTXT ("DC_miss_L2_hit"), GTXT ("hit own L2"));
      fprintf (f_usage, NTXT ("            %-22s %s\n"),
	       NTXT ("DC_miss_L3_hit"), GTXT ("hit own L3"));
      fprintf (f_usage, NTXT ("            %-22s %s\n"),
	       NTXT ("DC_miss_L3_dirty_copyback"), GTXT ("hit own L3 but require copyback from L2D"));
      fprintf (f_usage, NTXT ("            %-22s %s\n"),
	       NTXT ("DC_miss_nbr_L3_hit"), GTXT ("hit neighbor L3 (same socket)"));
      fprintf (f_usage, GTXT ("        Loads that miss all caches on this socket can be counted:\n"));
      fprintf (f_usage, NTXT ("            %-22s %s\n"),
	       NTXT ("DC_miss_remote_L3_hit"), GTXT ("hit cache on different socket"));
      fprintf (f_usage, NTXT ("            %-22s %s\n"),
	       NTXT ("DC_miss_local_mem_hit"), GTXT ("hit local memory (same socket)"));
      fprintf (f_usage, NTXT ("            %-22s %s\n"),
	       NTXT ("DC_miss_remote_mem_hit"), GTXT ("hit remote memory (off socket)"));
      fprintf (f_usage, GTXT ("        These events are for speculative loads, launched in anticipation\n"
			      "        of helping performance but whose results might not be committed.\n"));
#if 0 // was: #if defined(linux).  See 22236226 - sparc-Linux: Support basic Memoryspace and Dataspace profiling (capture VADDR)
      /* 21869427 should not look like memoryspace profiling is supported on Linux */
      /* 21869424 desire memoryspace profiling on Linux */
      fprintf (f_usage, GTXT ("        To count only data-cache misses that commit, use:\n"));
      fprintf (f_usage, NTXT ("            DC_miss_commit\n"));
#else
      fprintf (f_usage, GTXT ("        To count only data-cache misses that commit, or for memoryspace profiling,\n"
			      "        use the 'memoryspace' counter:\n"));
      fprintf (f_usage, NTXT ("            DC_miss_commit\n"));
#endif
      fprintf (f_usage, NTXT ("\n"));
    }
  else
    {
      fprintf (f_usage, GTXT ("        Total data-cache misses can be counted with:\n"));
      fprintf (f_usage, NTXT ("            DC_miss                DC_miss_nospec\n"));
      fprintf (f_usage, GTXT ("        They are the totals of misses that hit in L2/L3 cache, local memory, or\n"
			      "        remote memory:\n"));
      fprintf (f_usage, NTXT ("            DC_miss_L2_L3_hit      DC_miss_L2_L3_hit_nospec\n"));
      fprintf (f_usage, NTXT ("            DC_miss_local_hit      DC_miss_local_hit_nospec\n"));
      fprintf (f_usage, NTXT ("            DC_miss_remote_L3_hit  DC_miss_remote_L3_hit_nospec\n"));
      fprintf (f_usage, GTXT ("        The events in the left column include speculative operations.  Use the\n"
			      "        right-hand _nospec events to count only data accesses that commit\n"
			      "        or for memoryspace profiling.\n\n"));
    }

  fprintf (f_usage, GTXT ("        == TLB misses.\n"
			  "        The Translation Lookaside Buffer (TLB) is a cache of virtual-to-physical\n"
			  "        page translations."));
  fprintf (f_usage, GTXT ("  If a virtual address (VA) is not represented in the\n"
			  "        TLB, an expensive hardware table walk (HWTW) must be conducted."));
  fprintf (f_usage, GTXT ("  If the\n"
			  "        page is still not found, a trap results.  There is a data TLB (DTLB) and\n"
			  "        an instruction TLB (ITLB).\n\n"));
  fprintf (f_usage, GTXT ("        TLB misses can be counted by:\n"));
  fprintf (f_usage, NTXT ("            %s\n"),
	   cpuver == CPC_SPARC_M7 ?
	   NTXT ("DTLB_HWTW_search            ITLB_HWTW_search") :
	   cpuver == CPC_SPARC_M8 ?
	   NTXT ("DTLB_HWTW                   ITLB_HWTW") :
	   NTXT ("DTLB_miss_asynch            ITLB_miss_asynch"));
  fprintf (f_usage, GTXT ("        or broken down by page size:\n"));
  fprintf (f_usage, NTXT ("            %s"),
	   cpuver == CPC_SPARC_M7 ?
	   NTXT ("DTLB_HWTW_hit_8K            ITLB_HWTW_hit_8K\n"
		 "            DTLB_HWTW_hit_64K           ITLB_HWTW_hit_64K\n"
		 "            DTLB_HWTW_hit_4M            ITLB_HWTW_hit_4M\n") :
	   NTXT ("DTLB_fill_8KB               ITLB_fill_8KB\n"
		 "            DTLB_fill_64KB              ITLB_fill_64KB\n"
		 "            DTLB_fill_4MB               ITLB_fill_4MB\n"));
  fprintf (f_usage, NTXT ("            %s\n\n"),
	   cpuver == CPC_SPARC_M7 ?
	   NTXT ("DTLB_HWTW_hit_256M          ITLB_HWTW_hit_256M\n"
		 "            DTLB_HWTW_hit_2G_16G        ITLB_HWTW_hit_2G_16G\n"
		 "            DTLB_HWTW_miss_trap         ITLB_HWTW_miss_trap") :
	   cpuver == CPC_SPARC_M8 ?
	   NTXT ("DTLB_HWTW_hit_256M          ITLB_HWTW_hit_256M\n"
		 "            DTLB_HWTW_hit_16G           ITLB_HWTW_hit_16G\n"
		 "            DTLB_HWTW_hit_1T            ITLB_HWTW_hit_1T") :
	   NTXT ("DTLB_fill_256MB             ITLB_fill_256MB\n"
		 "            DTLB_fill_2GB               ITLB_fill_2GB\n"
		 "            DTLB_fill_trap              ITLB_fill_trap"));
  if (cpuver == CPC_SPARC_M8)
    {
      fprintf (f_usage, GTXT ("        TLB traps, which can require hundreds of cycles, can be counted with:\n"));
      fprintf (f_usage, NTXT ("            %s\n\n"),
	       NTXT ("DTLB_fill_trap              ITLB_fill_trap"));
    }

  fprintf (f_usage, GTXT ("        == Branch misprediction.\n"
			  "        Count branch mispredictions with:\n"
			  "            Br_mispred\n"
			  "        It is the total of:\n"
			  "            Br_dir_mispred         direction was mispredicted\n"
			  "            %s         target    was mispredicted\n"
			  "\n"), cpuver == CPC_SPARC_M7 ? NTXT ("Br_tgt_mispred") : NTXT ("Br_trg_mispred"));

  fprintf (f_usage, GTXT ("        == RAW hazards.\n"
			  "        A read-after-write (RAW) delay occurs when we attempt to read a datum\n"
			  "        before an earlier write has had time to complete:\n"));
  if (cpuver == CPC_SPARC_M8)
    {
      fprintf (f_usage, NTXT ("            RAW_hit\n"));
      fprintf (f_usage, GTXT ("        RAW_hit events can be broken down into:\n"));
    }
  else
    {
      fprintf (f_usage, NTXT ("            RAW_hit_st_q~emask=0xf\n"));
      fprintf (f_usage, GTXT ("        The mask 0xf counts the total of all types such as:\n"));
    }
  fprintf (f_usage, NTXT ("            RAW_hit_st_buf         write is still in store buffer\n"
			  "            RAW_hit_st_q           write is still in store queue\n"
			  "\n"));
  if (cpuver == CPC_SPARC_M7)
    {
      fprintf (f_usage, GTXT ("        == Flush.\n"
			      "        One can count the number of times the pipeline must be flushed:\n"));
      fprintf (f_usage, NTXT ("            %-22s %s\n"),
	       NTXT ("Flush_L3_miss"), GTXT ("load missed L3 and >1 strand is active on the core"));
      fprintf (f_usage, NTXT ("            %-22s %s\n"),
	       NTXT ("Flush_br_mispred"), GTXT ("branch misprediction"));
      fprintf (f_usage, NTXT ("            %-22s %s\n"),
	       NTXT ("Flush_arch_exception"), GTXT ("SPARC exceptions and trap entry/return"));
      fprintf (f_usage, NTXT ("            %-22s %s\n"),
	       NTXT ("Flush_other"), GTXT ("state change to/from halted/paused"));
      fprintf (f_usage, NTXT ("\n"));
    }
}

static void
hwc_usage_internal (int forKernel, FILE *f_usage, const char *cmd, const char *dataspace_msg, int show_syntax, int show_short_desc)
{
  if (!VALID_FOR_KERNEL (forKernel))
    return;
  char cpuname[128];
  hwc_get_cpuname (cpuname, 128);
  Hwcentry** raw_ctrs = hwc_get_raw_ctrs (forKernel);
  int has_raw_ctrs = (raw_ctrs && raw_ctrs[0]);
  Hwcentry** std_ctrs = hwc_get_std_ctrs (forKernel);
  int has_std_ctrs = (std_ctrs && std_ctrs[0]);
  unsigned hwc_maxregs = hwc_get_max_concurrent (forKernel);
  int cpuver = hwc_get_cpc_cpuver ();
  if (hwc_maxregs != 0)
    {
      if (show_syntax)
	{
	  fprintf (f_usage, GTXT ("\nSpecifying HW counters on `%s' (cpuver=%d):\n\n"), cpuname, cpuver);
	  fprintf (f_usage, GTXT ("    -h {auto|lo|on|hi}\n"));
	  fprintf (f_usage, GTXT ("\tturn on default set of HW counters at the specified rate\n"));
	  if (hwc_maxregs == 1)
	    {
	      fprintf (f_usage, GTXT ("    -h <ctr_def>\n"));
	      fprintf (f_usage, GTXT ("\tspecify HW counter profiling for one HW counter only\n"));
	    }
	  else
	    {
	      fprintf (f_usage, GTXT ("    -h <ctr_def> [-h <ctr_def>]...\n"));
	      fprintf (f_usage, GTXT ("    -h <ctr_def>[,<ctr_def>]...\n"));
	      fprintf (f_usage, GTXT ("\tspecify HW counter profiling for up to %u HW counters\n"), hwc_maxregs);
	    }
	  fprintf (f_usage, NTXT ("\n"));
	}
      else
	{
	  fprintf (f_usage, GTXT ("\nSpecifying HW counters on `%s' (cpuver=%d)\n\n"), cpuname, cpuver);
	  if (hwc_maxregs == 1)
	    fprintf (f_usage, GTXT ("           Hardware counter profiling is supported for only one counter.\n"));
	  else
	    fprintf (f_usage, GTXT ("           Hardware counter profiling is supported for up to %u HW counters.\n"), hwc_maxregs);
	}
    }
  else
    {
      if (!IS_KERNEL (forKernel))
	{ // EUGENE I don't see why we don't also use this for er_kernel
	  char buf[1024];
	  *buf = 0;
	  char *pch = hwcfuncs_errmsg_get (buf, sizeof (buf), 0);
	  if (*pch)
	    fprintf (f_usage, GTXT ("HW counter profiling is not supported on this system: %s%s"),
		     pch, pch[strlen (pch) - 1] == '\n' ? "" : "\n");
	  else
	    fprintf (f_usage, GTXT ("HW counter profiling is not supported on this system\n"));
	}
      return;
    }

  /* At this point, we know we have counters */
  char**hwc_attrs = hwc_get_attrs (forKernel);
  int has_attrs = (hwc_attrs && hwc_attrs[0]);
  if (show_syntax)
    {
      const char *reg_s = show_regs ? "[/<reg#>]" : "";
      const char *attr_s = has_attrs ? "[[~<attr>=<val>]...]" : "";
      fprintf (f_usage, GTXT ("    <ctr_def> == <ctr>%s%s,[<rate>]\n"), attr_s, reg_s);
      if (dataspace_msg)
	fprintf (f_usage, NTXT ("%s"), dataspace_msg);
      fprintf (f_usage, GTXT ("        <ctr>\n"));
      fprintf (f_usage, GTXT ("           counter name, "));
    }
  else
    fprintf (f_usage, GTXT ("           Counter name "));
  fprintf (f_usage, GTXT ("must be selected from the available counters\n"
			  "           listed below.  On most systems, if a counter is not listed\n"
			  "           below, it may still be specified by its numeric value.\n"));
  if (cpcx_has_precise[forKernel])
    {
      if (!forKernel)
	fprintf (f_usage, GTXT ("           Counters labeled as 'memoryspace' in the list below will\n"
				"           collect memoryspace data by default.\n"));
    }
  fprintf (f_usage, GTXT ("\n"));
  if (has_attrs)
    {
      if (show_syntax)
	{
	  fprintf (f_usage, GTXT ("        ~<attr>=<val>\n"));
	  fprintf (f_usage, GTXT ("           optional attribute where <val> can be in decimal or hex\n"
				  "           format, and <attr> can be one of: \n"));
	}
      else
	fprintf (f_usage, GTXT ("           Optional attribute where <val> can be in decimal or hex\n"
				"           format, and <attr> can be one of: \n"));
      for (char **pattr = hwc_attrs; *pattr; pattr++)
	fprintf (f_usage, NTXT ("             `%s'\n"), *pattr);
      if (show_syntax)
	fprintf (f_usage, GTXT ("           Multiple attributes may be specified, and each must be preceded by a ~.\n\n"));
      else
	fprintf (f_usage, GTXT ("           Multiple attributes may be specified.\n\n"));
      if (IS_KERNEL (forKernel))
	fprintf (f_usage, GTXT ("           Other attributes may be supported by the chip, but are not supported by DTrace and will be ignored by er_kernel.\n\n"));
    }

  if (show_syntax)
    {
      if (show_regs)
	fprintf (f_usage, GTXT ("        /<reg#>\n"
				"           forces use of a specific hardware register.  (Solaris only)\n"
				"           If not specified, %s will attempt to place the counter into the first\n"
				"           available register and as a result may be unable to place\n"
				"           subsequent counters due to register conflicts.\n"
				"           The / in front of the register number is required if a register is specified.\n\n"),
		 cmd);

      fprintf (f_usage, GTXT ("        <rate> == {auto|lo|on|hi}\n"));
      fprintf (f_usage, GTXT ("           `auto'   (default) match the rate used by clock profiling.\n"));
      fprintf (f_usage, GTXT ("                    If clock profiling is disabled, use `on'.\n"));
      fprintf (f_usage, GTXT ("           `lo'     per-thread maximum rate of ~10 samples/second\n"));
      fprintf (f_usage, GTXT ("           `on'     per-thread maximum rate of ~100 samples/second\n"));
      fprintf (f_usage, GTXT ("           `hi'     per-thread maximum rate of ~1000 samples/second\n\n"));
      fprintf (f_usage, GTXT ("        <rate> == <interval>\n"
			      "           Fixed event interval value to trigger a sample.\n"
			      "           Smaller intervals imply more frequent samples.\n"
			      "           Example: when counting cycles on a 2 GHz processor,\n"
			      "           an interval of 2,000,003 implies ~1000 samples/sec\n"
			      "\n"
			      "           Use this feature with caution, because:\n"
			      "             (1) Frequent sampling increases overhead and may disturb \n"
			      "                 other applications on your system.\n"
			      "             (2) Event counts vary dramatically depending on the event \n"
			      "                 and depending on the application.\n"
			      "             (3) A fixed event interval disables any other gprofng\n"
			      "                 internal mechanisms that may limit event rates.\n"
			      "\n"
			      "           Guidelines:  Aim at <1000 events per second.  Start by \n"
			      "           collecting with the 'hi' option; in the experiment overview,\n"
			      "           notice how many events are recorded per second; divide by\n"
			      "           1000, and use that as your starting point.\n\n"));

      fprintf (f_usage, GTXT ("        A comma ',' followed immediately by white space may be omitted.\n\n"));
    }

  /* default counters */
  fprintf (f_usage, GTXT ("Default set of HW counters:\n\n"));
  char * defctrs = hwc_get_default_cntrs2 (forKernel, 1);
  if (defctrs == NULL)
    fprintf (f_usage, GTXT ("    No default HW counter set defined for this system.\n"));
  else if (strlen (defctrs) == 0)
    {
      char *s = hwc_get_orig_default_cntrs (forKernel);
      fprintf (f_usage, GTXT ("    The default HW counter set (%s) defined for %s cannot be loaded on this system.\n"),
	       s, cpuname);
      free (s);
      free (defctrs);
    }
  else
    {
      char *defctrs2 = hwc_get_default_cntrs2 (forKernel, 2);
      fprintf (f_usage, GTXT ("    -h %s\n"), defctrs);
      free (defctrs2);
      free (defctrs);
    }

  /* long listings */
  char tmp[1024];
  if (has_std_ctrs)
    {
      fprintf (f_usage, GTXT ("\nAliases for most useful HW counters:\n\n"));
      format_columns (tmp, 1024, "alias", "raw name", "type ", "units", "regs", "description");
      fprintf (f_usage, NTXT ("    %s\n\n"), tmp);
      for (Hwcentry **pctr = std_ctrs; *pctr; pctr++)
	{
	  Hwcentry *ctr = *pctr;
	  hwc_hwcentry_string_internal (tmp, sizeof (tmp), ctr, 0);
	  fprintf (f_usage, NTXT ("    %s\n"), tmp);
	}
    }
  if (has_raw_ctrs)
    {
      fprintf (f_usage, GTXT ("\nRaw HW counters:\n\n"));
      hwc_usage_raw_overview_sparc (f_usage, cpuver);
      format_columns (tmp, 1024, "name", NULL, "type ", "units", "regs", "description");
      fprintf (f_usage, NTXT ("    %s\n\n"), tmp);
      for (Hwcentry **pctr = raw_ctrs; *pctr; pctr++)
	{
	  Hwcentry *ctr = *pctr;
	  hwc_hwcentry_string_internal (tmp, sizeof (tmp), ctr, show_short_desc);
	  fprintf (f_usage, NTXT ("    %s\n"), tmp);
	}
    }

  /* documentation notice */
  hwc_get_docref (tmp, 1024);
  if (strlen (tmp))
    fprintf (f_usage, NTXT ("\n%s\n"), tmp);
}

/* Print a description of "-h" usage, largely common to collect and er_kernel. */
void
hwc_usage (int forKernel, const char *cmd, const char *dataspace_msg)
{
  hwc_usage_internal (forKernel, stdout, cmd, dataspace_msg, 1, 0);
}

void
hwc_usage_f (int forKernel, FILE *f, const char *cmd, const char *dataspace_msg, int show_syntax, int show_short_desc)
{
  hwc_usage_internal (forKernel, f, cmd, dataspace_msg, show_syntax, show_short_desc);
}

/*---------------------------------------------------------------------------*/
/* init functions */

static char* supported_pebs_counters[] = {
  "mem_inst_retired.latency_above_threshold",
  "mem_trans_retired.load_latency",
  "mem_trans_retired.precise_store",
  NULL
};

/* callback, (see setup_cpc()) called for each valid regno/name combo */

/* builds rawlist,, creates and updates reg_list[] arrays in stdlist table */
static void
hwc_cb (uint_t cpc_regno, const char *name)
{
  regno_t regno = cpc_regno; /* convert type */
  list_add (&unfiltered_raw, regno, name);
}

/* input:
 *   forKernel: 1 - generate lists for er_kernel, 0 - generate lists for collect
 *
 *   raw_orig: HWCs as generated by hwc_cb()
 * output:
 *   pstd_out[], praw_out[]: malloc'd array of pointers to malloc'd hwcentry, or NULL
 */
static void
hwc_process_raw_ctrs (int forKernel, Hwcentry ***pstd_out,
		      Hwcentry ***praw_out, Hwcentry ***phidden_out,
		      Hwcentry**static_tables, Hwcentry **raw_unfiltered_in)
{
  // set up output buffers
  ptr_list s_outbufs[3];
  ptr_list *std_out = &s_outbufs[0];
  ptr_list_init (std_out);
  ptr_list *raw_out = &s_outbufs[1];
  ptr_list_init (raw_out);
  ptr_list *hidden_out = &s_outbufs[2];
  ptr_list_init (hidden_out);

#define NUM_TABLES 3
  ptr_list table_copy[NUM_TABLES]; // copy of data from static tables. [0]std, [1]generic, and [2]hidden
  for (int tt = 0; tt < NUM_TABLES; tt++)
    ptr_list_init (&table_copy[tt]);

  // copy records from std [0] and generic [1] static input tables into table_copy[0],[1],or[2]
  for (int tt = 0; tt < 2; tt++)
    for (Hwcentry *pctr = static_tables[tt]; pctr && pctr->name; pctr++)
      if (is_hidden_alias (pctr))
	list_append_shallow_copy (&table_copy[2], pctr); // hidden list
      else
	list_append_shallow_copy (&table_copy[tt], pctr);

  // copy raw_unfiltered_in to raw_out
  for (int ii = 0; raw_unfiltered_in && raw_unfiltered_in[ii]; ii++)
    {
      Hwcentry *pctr = raw_unfiltered_in[ii];
      // filter out raw counters that don't work correctly

#ifdef WORKAROUND_6231196_NIAGARA1_NO_CTR_0
      if (cpcx_cpuver == CPC_ULTRA_T1)
	if (!regno_is_valid (pctr, 1))
	  continue;   /* Niagara can not profile on register zero; skip this */
#endif
      // remove specific PEBs counters when back end doesn't support sampling
      const char *name = pctr->name;
      if ((cpcx_support_bitmask & HWCFUNCS_SUPPORT_PEBS_SAMPLING) == 0 || forKernel)
	{
	  int skip = 0;
	  for (int ii = 0; supported_pebs_counters[ii]; ii++)
	    if (strcmp (supported_pebs_counters[ii], name) == 0)
	      {
		skip = 1;
		break;
	      }
	  if (skip)
	    continue;
	}

      Hwcentry *pnew = list_append_shallow_copy (raw_out, pctr);
#ifdef WORKAROUND_6231196_NIAGARA1_NO_CTR_0
      if (cpcx_cpuver == CPC_ULTRA_T1)
	{
	  free (pnew->reg_list);
	  pnew->reg_list = NULL;
	  regno_add (pnew, 1); // only allow register 1
	}
#endif
    } // raw_unfiltered_in

  // Scan raw counters to populate Hwcentry fields from matching static_tables entries
  // Also populate reg_list for aliases found in table_copy[]
  for (int uu = 0; uu < raw_out->sz; uu++)
    {
      Hwcentry *praw = (Hwcentry*) raw_out->array[uu];
      Hwcentry *pstd = NULL; // set if non-alias entry from std table matches
      char *name = praw->name;
      /* in the standard counter and generic lists,
	 update reg_list for all matching items  */
      for (int tt = 0; tt < NUM_TABLES; tt++)
	{ // std, generic, and hidden
	  if (table_copy[tt].sz == 0)
	    continue;
	  Hwcentry **array = (Hwcentry**) table_copy[tt].array;
	  for (int jj = 0; array[jj]; jj++)
	    { // all table counters
	      Hwcentry *pctr = array[jj];
	      char *pname;
	      if (pctr->int_name)
		pname = pctr->int_name;
	      else
		pname = pctr->name;
	      if (!is_same (name, pname, '~'))
		continue;

	      /* truncated pname matches <name>... */
	      // check to see if table entry applies only to specific register
	      int specific_reg_num_only = 0;
	      if (pctr->reg_num != REGNO_ANY)
		{
		  // table entry applies only to specific register
		  if (!regno_is_valid (praw, pctr->reg_num))
		    continue;
		  specific_reg_num_only = 1;
		}

	      // Match!
	      // Update cpu_table_copy's supported registers
	      if (specific_reg_num_only)
		regno_add (pctr, pctr->reg_num);
	      else
		pctr->reg_list = praw->reg_list;

	      if (!is_visible_alias (pctr) && !is_hidden_alias (pctr))
		{
		  // Note: we could expand criteria to also allow aliases to set default rates for raw HWCs
		  /* This is an 'internal' raw counter */
		  if (!pstd)
		    pstd = pctr; /* use info as a template when adding to raw list */
		  else
		    hwcentry_print (DBG_LT0, "hwctable: hwc_cb: Warning: "
				    "counter %s appears in table more than once: ",
				    pstd);
		}
	    }/* for table rows */
	}/* for std and generic tables */

      if (pstd)
	{
	  /* the main table had an entry that matched <name> exactly */
	  /* Apply the main table entry as a template */
	  *praw = *pstd;
	}
    }/* for (raw_out) */

  // update std_out and hidden_out
  for (int tt = 0; tt < NUM_TABLES; tt++)
    {
      if (tt == 1 /*skip std_raw*/ || table_copy[tt].sz == 0)
	continue;
      Hwcentry *pctr;
      for (int ii = 0; (pctr = table_copy[tt].array[ii]); ii++)
	{
	  // prune unsupported rows from std table
	  if (!is_visible_alias (pctr) && !is_hidden_alias (pctr))
	    continue; // only aliases
	  if (REG_LIST_IS_EMPTY (pctr->reg_list))
	    {
	      if (is_numeric_alias (pctr))
		{
#if 1 //22844570 DTrace cpc provider does not accept numeric counter names
		  if (forKernel)
		    continue;
#endif
		  regno_add (pctr, REGNO_ANY); // hwcs specified by number allowed on any register
		}
	      else
		continue;
	    }

	  ptr_list *dest = (tt == 0) ? std_out : hidden_out;
	  Hwcentry *isInList;
	  if (pctr->short_desc == NULL)
	    {
	      isInList = ptrarray_find_by_name ((Hwcentry**) raw_out->array, pctr->int_name);
	      if (isInList)
		pctr->short_desc = isInList->short_desc; // copy the raw counter's detailed description
	    }
	  isInList = ptrarray_find_by_name ((Hwcentry**) dest->array, pctr->name);
	  if (isInList)
	    hwcentry_print (DBG_LT0, "hwctable: hwc_cb: Warning: "
			    "counter %s appears in alias list more than once: ",
			    pctr);
	  else
	    list_append_shallow_copy (dest, pctr);
	}
    }
  for (int tt = 0; tt < NUM_TABLES; tt++)
    ptr_list_free (&table_copy[tt]);

  if (forKernel)
    {
      // for er_kernel, use baseline value of PRELOAD_DEF_ERKERNEL instead of PRELOAD_DEF
      for (int tt = 0; tt < 3; tt++)
	{ // std_out-0, raw_out-1, hidden_out-2
	  Hwcentry** hwcs = (Hwcentry**) (s_outbufs[tt].array);
	  for (int ii = 0; hwcs && hwcs[ii]; ii++)
	    {
	      Hwcentry *hwc = hwcs[ii];
	      if (hwc->val == PRELOAD_DEF)
		hwc->val = PRELOAD_DEF_ERKERNEL;
	    }
	}
    }
  *pstd_out = (Hwcentry**) std_out->array;
  *praw_out = (Hwcentry**) raw_out->array;
  *phidden_out = (Hwcentry**) hidden_out->array;
}

/* callback, (see setup_cpc()) called for each valid attribute */
/* builds attrlist */
static void
attrs_cb (const char *attr)
{
  Tprintf (DBG_LT3, "hwctable: attrs_cb(): %s\n", attr);
  if (strcmp (attr, "picnum") == 0)
    return;     /* don't make this attribute available to users */
  ptr_list_add (&unfiltered_attrs, (void*) strdup (attr));
}

/* returns true if attribute is valid for this platform */
static int
attr_is_valid (int forKernel, const char *attr)
{
  setup_cpcx ();
  if (!VALID_FOR_KERNEL (forKernel) || !cpcx_attrs[forKernel])
    return 0;
  for (int ii = 0; cpcx_attrs[forKernel][ii]; ii++)
    if (strcmp (attr, cpcx_attrs[forKernel][ii]) == 0)
      return 1;
  return 0;
}
