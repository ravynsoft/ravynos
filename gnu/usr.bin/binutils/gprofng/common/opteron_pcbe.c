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

/*
 * This file contains preset event names from the Performance Application
 * Programming Interface v3.5 which included the following notice:
 *
 *                             Copyright (c) 2005,6
 *                           Innovative Computing Labs
 *                         Computer Science Department,
 *                            University of Tennessee,
 *                                 Knoxville, TN.
 *                              All Rights Reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    * Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
 *    * Neither the name of the University of Tennessee nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *	this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * This open source software license conforms to the BSD License template.
 */

/*
 * Performance Counter Back-End for AMD Opteron and AMD Athlon 64 processors.
 */

#include <sys/types.h>
#include "hwcdrv.h"

#define CPU /* used by cpuid_get*() functions */

typedef struct _amd_event
{
  char *name;
  uint16_t emask;       /* Event mask setting */
  uint8_t umask_valid;  /* Mask of unreserved UNIT_MASK bits */
} amd_event_t;

typedef struct _amd_generic_event
{
  char *name;
  char *event;
  uint8_t umask;
} amd_generic_event_t;

#define EV_END      { NULL, 0, 0 }
#define GEN_EV_END  { NULL, NULL, 0 }

#define AMD_cmn_events       \
  { "FP_dispatched_fpu_ops",                            0x00, 0x3F }, \
  { "FP_cycles_no_fpu_ops_retired",                     0x01, 0x0 }, \
  { "FP_dispatched_fpu_ops_ff",                         0x02, 0x0 }, \
  { "LS_seg_reg_load",                                  0x20, 0x7F }, \
  { "LS_uarch_resync_self_modify",                      0x21, 0x0 }, \
  { "LS_uarch_resync_snoop",                            0x22, 0x0 }, \
  { "LS_buffer_2_full",                                 0x23, 0x0 }, \
  { "LS_retired_cflush",                                0x26, 0x0 }, \
  { "LS_retired_cpuid",                                 0x27, 0x0 }, \
  { "DC_access",                                        0x40, 0x0 }, \
  { "DC_miss",                                          0x41, 0x0 }, \
  { "DC_refill_from_L2",                                0x42, 0x1F }, \
  { "DC_refill_from_system",                            0x43, 0x1F }, \
  { "DC_misaligned_data_ref",                           0x47, 0x0 }, \
  { "DC_uarch_late_cancel_access",                      0x48, 0x0 }, \
  { "DC_uarch_early_cancel_access",                     0x49, 0x0 }, \
  { "DC_dispatched_prefetch_instr",                     0x4B, 0x7 }, \
  { "DC_dcache_accesses_by_locks",                      0x4C, 0x2 }, \
  { "BU_memory_requests",                               0x65, 0x83}, \
  { "BU_data_prefetch",                                 0x67, 0x3 }, \
  { "BU_cpu_clk_unhalted",                              0x76, 0x0 }, \
  { "IC_fetch",                                         0x80, 0x0 }, \
  { "IC_miss",                                          0x81, 0x0 }, \
  { "IC_refill_from_L2",                                0x82, 0x0 }, \
  { "IC_refill_from_system",                            0x83, 0x0 }, \
  { "IC_itlb_L1_miss_L2_hit",                           0x84, 0x0 }, \
  { "IC_uarch_resync_snoop",                            0x86, 0x0 }, \
  { "IC_instr_fetch_stall",                             0x87, 0x0 }, \
  { "IC_return_stack_hit",                              0x88, 0x0 }, \
  { "IC_return_stack_overflow",                         0x89, 0x0 }, \
  { "FR_retired_x86_instr_w_excp_intr",                 0xC0, 0x0 }, \
  { "FR_retired_uops",                                  0xC1, 0x0 }, \
  { "FR_retired_branches_w_excp_intr",                  0xC2, 0x0 }, \
  { "FR_retired_branches_mispred",                      0xC3, 0x0 }, \
  { "FR_retired_taken_branches",                        0xC4, 0x0 }, \
  { "FR_retired_taken_branches_mispred",                0xC5, 0x0 }, \
  { "FR_retired_far_ctl_transfer",                      0xC6, 0x0 }, \
  { "FR_retired_resyncs",                               0xC7, 0x0 }, \
  { "FR_retired_near_rets",                             0xC8, 0x0 }, \
  { "FR_retired_near_rets_mispred",                     0xC9, 0x0 }, \
  { "FR_retired_taken_branches_mispred_addr_miscomp",   0xCA, 0x0 }, \
  { "FR_retired_fastpath_double_op_instr",              0xCC, 0x7 }, \
  { "FR_intr_masked_cycles",                            0xCD, 0x0 }, \
  { "FR_intr_masked_while_pending_cycles",              0xCE, 0x0 }, \
  { "FR_taken_hardware_intrs",                          0xCF, 0x0 }, \
  { "FR_nothing_to_dispatch",                           0xD0, 0x0 }, \
  { "FR_dispatch_stalls",                               0xD1, 0x0 }, \
  { "FR_dispatch_stall_branch_abort_to_retire",         0xD2, 0x0 }, \
  { "FR_dispatch_stall_serialization",                  0xD3, 0x0 }, \
  { "FR_dispatch_stall_segment_load",                   0xD4, 0x0 }, \
  { "FR_dispatch_stall_reorder_buffer_full",            0xD5, 0x0 }, \
  { "FR_dispatch_stall_resv_stations_full",             0xD6, 0x0 }, \
  { "FR_dispatch_stall_fpu_full",                       0xD7, 0x0 }, \
  { "FR_dispatch_stall_ls_full",                        0xD8, 0x0 }, \
  { "FR_dispatch_stall_waiting_all_quiet",              0xD9, 0x0 }, \
  { "FR_dispatch_stall_far_ctl_trsfr_resync_branch_pend", 0xDA, 0x0 },\
  { "FR_fpu_exception",                                 0xDB, 0xF }, \
  { "FR_num_brkpts_dr0",                                0xDC, 0x0 }, \
  { "FR_num_brkpts_dr1",                                0xDD, 0x0 }, \
  { "FR_num_brkpts_dr2",                                0xDE, 0x0 }, \
  { "FR_num_brkpts_dr3",                                0xDF, 0x0 }, \
  { "NB_mem_ctrlr_bypass_counter_saturation",           0xE4, 0xF }

#define OPT_events \
  { "LS_locked_operation",                              0x24, 0x7 }, \
  { "DC_copyback",                                      0x44, 0x1F }, \
  { "DC_dtlb_L1_miss_L2_hit",                           0x45, 0x0 }, \
  { "DC_dtlb_L1_miss_L2_miss",                          0x46, 0x0 }, \
  { "DC_1bit_ecc_error_found",                          0x4A, 0x3 }, \
  { "BU_system_read_responses",                         0x6C, 0x7 }, \
  { "BU_quadwords_written_to_system",                   0x6D, 0x1 }, \
  { "BU_internal_L2_req",                               0x7D, 0x1F }, \
  { "BU_fill_req_missed_L2",                            0x7E, 0x7 }, \
  { "BU_fill_into_L2",                                  0x7F, 0x1 }, \
  { "IC_itlb_L1_miss_L2_miss",                          0x85, 0x0 }, \
  { "FR_retired_fpu_instr",                             0xCB, 0xF }, \
  { "NB_mem_ctrlr_page_access",                         0xE0, 0x7 }, \
  { "NB_mem_ctrlr_page_table_overflow",                 0xE1, 0x0 }, \
  { "NB_mem_ctrlr_turnaround",                          0xE3, 0x7 }, \
  { "NB_ECC_errors",                                    0xE8, 0x80}, \
  { "NB_sized_commands",                                0xEB, 0x7F }, \
  { "NB_probe_result",                                  0xEC, 0x7F}, \
  { "NB_gart_events",                                   0xEE, 0x7 }, \
  { "NB_ht_bus0_bandwidth",                             0xF6, 0xF }, \
  { "NB_ht_bus1_bandwidth",                             0xF7, 0xF }, \
  { "NB_ht_bus2_bandwidth",                             0xF8, 0xF }

#define OPT_RevD_events \
  { "NB_sized_blocks",                                  0xE5, 0x3C }

#define OPT_RevE_events \
  { "NB_cpu_io_to_mem_io",                              0xE9, 0xFF}, \
  { "NB_cache_block_commands",                          0xEA, 0x3D}

#define AMD_FAMILY_10h_cmn_events \
  { "FP_retired_sse_ops",                               0x3,   0x7F}, \
  { "FP_retired_move_ops",                              0x4,   0xF}, \
  { "FP_retired_serialize_ops",                         0x5,   0xF}, \
  { "FP_serialize_ops_cycles",                          0x6,   0x3}, \
  { "DC_copyback",                                      0x44,  0x7F }, \
  { "DC_dtlb_L1_miss_L2_hit",                           0x45,  0x3 }, \
  { "DC_dtlb_L1_miss_L2_miss",                          0x46,  0x7 }, \
  { "DC_1bit_ecc_error_found",                          0x4A,  0xF }, \
  { "DC_dtlb_L1_hit",                                   0x4D,  0x7 }, \
  { "BU_system_read_responses",                         0x6C,  0x17 }, \
  { "BU_octwords_written_to_system",                    0x6D,  0x1 }, \
  { "BU_internal_L2_req",                               0x7D,  0x3F }, \
  { "BU_fill_req_missed_L2",                            0x7E,  0xF }, \
  { "BU_fill_into_L2",                                  0x7F,  0x3 }, \
  { "IC_itlb_L1_miss_L2_miss",                          0x85,  0x3 }, \
  { "IC_eviction",                                      0x8B,  0x0 }, \
  { "IC_cache_lines_invalidate",                        0x8C,  0xF }, \
  { "IC_itlb_reload",                                   0x99,  0x0 }, \
  { "IC_itlb_reload_aborted",                           0x9A,  0x0 }, \
  { "FR_retired_mmx_sse_fp_instr",                      0xCB,  0x7 }, \
  { "NB_mem_ctrlr_page_access",                         0xE0,  0xFF }, \
  { "NB_mem_ctrlr_page_table_overflow",                 0xE1,  0x3 }, \
  { "NB_mem_ctrlr_turnaround",                          0xE3,  0x3F }, \
  { "NB_thermal_status",                                0xE8,  0x7C}, \
  { "NB_sized_commands",                                0xEB,  0x3F }, \
  { "NB_probe_results_upstream_req",                    0xEC,  0xFF}, \
  { "NB_gart_events",                                   0xEE,  0xFF }, \
  { "NB_ht_bus0_bandwidth",                             0xF6,  0xBF }, \
  { "NB_ht_bus1_bandwidth",                             0xF7,  0xBF }, \
  { "NB_ht_bus2_bandwidth",                             0xF8,  0xBF }, \
  { "NB_ht_bus3_bandwidth",                             0x1F9, 0xBF }, \
  { "LS_locked_operation",                              0x24,  0xF }, \
  { "LS_cancelled_store_to_load_fwd_ops",               0x2A,  0x7 }, \
  { "LS_smi_received",                                  0x2B,  0x0 }, \
  { "LS_ineffective_prefetch",                          0x52,  0x9 }, \
  { "LS_global_tlb_flush",                              0x54,  0x0 }, \
  { "NB_mem_ctrlr_dram_cmd_slots_missed",               0xE2,  0x3 }, \
  { "NB_mem_ctrlr_req",                                 0x1F0, 0xFF }, \
  { "CB_cpu_to_dram_req_to_target",                     0x1E0, 0xFF }, \
  { "CB_io_to_dram_req_to_target",                      0x1E1, 0xFF }, \
  { "CB_cpu_read_cmd_latency_to_target_0_to_3",         0x1E2, 0xFF }, \
  { "CB_cpu_read_cmd_req_to_target_0_to_3",             0x1E3, 0xFF }, \
  { "CB_cpu_read_cmd_latency_to_target_4_to_7",         0x1E4, 0xFF }, \
  { "CB_cpu_read_cmd_req_to_target_4_to_7",             0x1E5, 0xFF }, \
  { "CB_cpu_cmd_latency_to_target_0_to_7",              0x1E6, 0xFF }, \
  { "CB_cpu_req_to_target_0_to_7",                      0x1E7, 0xFF }, \
  { "L3_read_req",                                      0x4E0, 0xF7 }, \
  { "L3_miss",                                          0x4E1, 0xF7 }, \
  { "L3_l2_eviction_l3_fill",                           0x4E2, 0xFF }, \
  { "L3_eviction",                                      0x4E3, 0xF  }

#define AMD_cmn_generic_events \
  { "PAPI_br_ins", "FR_retired_branches_w_excp_intr",   0x0 },\
  { "PAPI_br_msp", "FR_retired_branches_mispred",       0x0 }, \
  { "PAPI_br_tkn", "FR_retired_taken_branches",         0x0 }, \
  { "PAPI_fp_ops", "FP_dispatched_fpu_ops",             0x3 }, \
  { "PAPI_fad_ins", "FP_dispatched_fpu_ops",            0x1 }, \
  { "PAPI_fml_ins", "FP_dispatched_fpu_ops",            0x2 }, \
  { "PAPI_fpu_idl", "FP_cycles_no_fpu_ops_retired",     0x0 }, \
  { "PAPI_tot_cyc", "BU_cpu_clk_unhalted",              0x0 }, \
  { "PAPI_tot_ins", "FR_retired_x86_instr_w_excp_intr", 0x0 }, \
  { "PAPI_l1_dca", "DC_access",                         0x0 }, \
  { "PAPI_l1_dcm", "DC_miss",                           0x0 }, \
  { "PAPI_l1_ldm", "DC_refill_from_L2",                 0xe }, \
  { "PAPI_l1_stm", "DC_refill_from_L2",                 0x10 }, \
  { "PAPI_l1_ica", "IC_fetch",                          0x0 }, \
  { "PAPI_l1_icm", "IC_miss",                           0x0 }, \
  { "PAPI_l1_icr", "IC_fetch",                          0x0 }, \
  { "PAPI_l2_dch", "DC_refill_from_L2",                 0x1e }, \
  { "PAPI_l2_dcm", "DC_refill_from_system",             0x1e }, \
  { "PAPI_l2_dcr", "DC_refill_from_L2",                 0xe }, \
  { "PAPI_l2_dcw", "DC_refill_from_L2",                 0x10 }, \
  { "PAPI_l2_ich", "IC_refill_from_L2",                 0x0 }, \
  { "PAPI_l2_icm", "IC_refill_from_system",             0x0 }, \
  { "PAPI_l2_ldm", "DC_refill_from_system",             0xe }, \
  { "PAPI_l2_stm", "DC_refill_from_system",             0x10 }, \
  { "PAPI_res_stl", "FR_dispatch_stalls",               0x0 }, \
  { "PAPI_stl_icy", "FR_nothing_to_dispatch",           0x0 }, \
  { "PAPI_hw_int", "FR_taken_hardware_intrs",           0x0 }

#define OPT_cmn_generic_events \
  { "PAPI_tlb_dm", "DC_dtlb_L1_miss_L2_miss",           0x0 }, \
  { "PAPI_tlb_im", "IC_itlb_L1_miss_L2_miss",           0x0 }, \
  { "PAPI_fp_ins", "FR_retired_fpu_instr",              0xd }, \
  { "PAPI_vec_ins", "FR_retired_fpu_instr",             0x4 }

#define AMD_FAMILY_10h_generic_events \
  { "PAPI_tlb_dm", "DC_dtlb_L1_miss_L2_miss",           0x7 }, \
  { "PAPI_tlb_im", "IC_itlb_L1_miss_L2_miss",           0x3 }, \
  { "PAPI_l3_dcr", "L3_read_req",                       0xf1 }, \
  { "PAPI_l3_icr", "L3_read_req",                       0xf2 }, \
  { "PAPI_l3_tcr", "L3_read_req",                       0xf7 }, \
  { "PAPI_l3_stm", "L3_miss",                           0xf4 }, \
  { "PAPI_l3_ldm", "L3_miss",                           0xf3 }, \
  { "PAPI_l3_tcm", "L3_miss",                           0xf7 }

static amd_event_t opt_events_rev_E[] = {
  AMD_cmn_events,
  OPT_events,
  OPT_RevD_events,
  OPT_RevE_events,
  EV_END
};

static amd_event_t family_10h_events[] = {
  AMD_cmn_events,
  OPT_RevE_events,
  AMD_FAMILY_10h_cmn_events,
  EV_END
};

static amd_generic_event_t opt_generic_events[] = {
  AMD_cmn_generic_events,
  OPT_cmn_generic_events,
  GEN_EV_END
};

static amd_generic_event_t family_10h_generic_events[] = {
  AMD_cmn_generic_events,
  AMD_FAMILY_10h_generic_events,
  GEN_EV_END
};

static amd_event_t *amd_events = NULL;
static uint_t amd_family;
static amd_generic_event_t *amd_generic_events = NULL;

#define BITS(v, u, l)       (((v) >> (l)) & ((1 << (1 + (u) - (l))) - 1))
#define OPTERON_FAMILY      0x0f
#define AMD_FAMILY_10H      0x10

static int
opt_pcbe_init (void)
{
  amd_family = cpuid_getfamily ();
  /*
   * Make sure this really _is_ an Opteron or Athlon 64 system. The kernel
   * loads this module based on its name in the module directory, but it
   * could have been renamed.
   */
  if (cpuid_getvendor () != X86_VENDOR_AMD
      || (amd_family != OPTERON_FAMILY && amd_family != AMD_FAMILY_10H))
    return (-1);

  /*
   * Figure out processor revision here and assign appropriate
   * event configuration.
   */
  if (amd_family == OPTERON_FAMILY)
    {
      amd_events = opt_events_rev_E;
      amd_generic_events = opt_generic_events;
    }
  else
    {
      amd_events = family_10h_events;
      amd_generic_events = family_10h_generic_events;
    }
  return (0);
}

static uint_t
opt_pcbe_ncounters (void)
{
  return (4);
}

static const char *
opt_pcbe_impl_name (void)
{
  if (amd_family == OPTERON_FAMILY)
    return ("AMD Opteron & Athlon64");
  else if (amd_family == AMD_FAMILY_10H)
    return ("AMD Family 10h");
  else
    return ("Unknown AMD processor");
}

static const char *
opt_pcbe_cpuref (void)
{
  if (amd_family == OPTERON_FAMILY)
    return GTXT ("See Chapter 10 of the \"BIOS and Kernel Developer's Guide for the AMD Athlon 64 and AMD Opteron Processors,\"\nAMD publication #26094");
  else if (amd_family == AMD_FAMILY_10H)
    return GTXT ("See section 3.15 of the \"BIOS and Kernel Developer's Guide (BKDG) For AMD Family 10h Processors,\"\nAMD publication #31116");
  else
    return GTXT ("Unknown AMD processor");
}

static int
opt_pcbe_get_events (hwcf_hwc_cb_t *hwc_cb)
{
  int count = 0;
  for (uint_t kk = 0; amd_events && amd_events[kk].name; kk++)
    for (uint_t jj = 0; jj < opt_pcbe_ncounters (); jj++)
      {
	hwc_cb (jj, amd_events[kk].name);
	count++;
      }
  for (uint_t kk = 0; amd_generic_events && amd_generic_events[kk].name; kk++)
    for (uint_t jj = 0; jj < opt_pcbe_ncounters (); jj++)
      {
	hwc_cb (jj, amd_generic_events[kk].name);
	count++;
      }
  return count;
}

static int
opt_pcbe_get_eventnum (const char *eventname, uint_t pmc, eventsel_t *eventsel,
		       eventsel_t *event_valid_umask, uint_t *pmc_sel)
{
  uint_t kk;
  *pmc_sel = pmc; /* for AMD, pmc doesn't need to be adjusted */
  *eventsel = (eventsel_t) - 1;
  *event_valid_umask = 0x0;

  /* search table */
  for (kk = 0; amd_events && amd_events[kk].name; kk++)
    {
      if (strcmp (eventname, amd_events[kk].name) == 0)
	{
	  *eventsel = EXTENDED_EVNUM_2_EVSEL (amd_events[kk].emask);
	  *event_valid_umask = amd_events[kk].umask_valid;
	  return 0;
	}
    }

  /* search generic */
  int generic = 0;
  eventsel_t tmp_umask = 0;
  for (kk = 0; amd_generic_events && amd_generic_events[kk].name; kk++)
    {
      if (strcmp (eventname, amd_generic_events[kk].name) == 0)
	{
	  generic = 1;
	  eventname = amd_generic_events[kk].event;
	  tmp_umask = amd_generic_events[kk].umask;
	  break;
	}
    }
  if (!generic)
    return -1;

  /* find real event # for generic event */
  for (kk = 0; amd_events && amd_events[kk].name; kk++)
    {
      if (strcmp (eventname, amd_events[kk].name) == 0)
	{
	  *eventsel = EXTENDED_EVNUM_2_EVSEL (amd_events[kk].emask);
	  *eventsel |= (tmp_umask << PERFCTR_UMASK_SHIFT);
	  *event_valid_umask = 0; /* user umask not allowed w/generic events */
	  return 0;
	}
    }
  return -1;
}

static hdrv_pcbe_api_t hdrv_pcbe_opteron_api = {
  opt_pcbe_init,
  opt_pcbe_ncounters,
  opt_pcbe_impl_name,
  opt_pcbe_cpuref,
  opt_pcbe_get_events,
  opt_pcbe_get_eventnum
};
