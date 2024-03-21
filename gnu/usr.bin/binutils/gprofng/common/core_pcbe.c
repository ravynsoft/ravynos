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
 * Performance Counter Back-End for Intel Family 6
 *   Models 15(06_0FH) 23(06_17H)                       (Core 2)
 *   Models 28(06_1CH)                                  (Atom)
 *   Models 37(06_25H) 44(06_2CH)                       (Westmere)
 *   Models 26(06_1AH) 30(06_1EH) 31(06_1FH) 46(06_2EH) (Nehalem)
 *   Models 42(06_2AH) 45(06_2DH)                       (Sandy Bridge)
 *   Models 58(06_3AH) 62(06_3EH)                       (Ivy Bridge)
 *   Models 60(06_3CH) 63(06_3FH) 69(06_45H) 70(06_46H) (Haswell)
 *   Models 61(06_3DH) 71(06_47H) 79(06_4FH) 86(06_??H) (Broadwell) (79 not listed in Intel SDM as of June 2015)
 *   Models 78(06_4EH) 85(06_55H) 94(06_5EH)            (Skylake) (Note Skylake and later: versionID==4)
 * To add another model number:
 *   - add appropriate table data in the form
 *       #define EVENTS_FAM6_MODXX
 *   - add appropriate table definitions in the form
 *       const struct events_table_t events_fam6_modXX[] =
 *   - set events_table to the appropriate table
 *     using the "switch ( cpuid_getmodel(CPU) )" statement
 *     in core_pcbe_init()
 *   - check the date in core_pcbe_cpuref()
 * Table data can be derived from:
 *   - the Intel SDM
 *       also https://download.01.org/perfmon/
 *   - libcpc source code in usr/src/uts/intel/pcbe/
 *   - libpfm4
 * but there are typically inconsistencies among these
 * sources of data.  So, judgment is required.
 * Other things to do to add a new processor:
 *   x file hwc_cpus.h
 *     add a cpuver enumerator
 *     add lookup entry
 *   x file hwctable.c
 *     add a table (aliases, etc.)
 *     add a cputabs entry, including default metrics
 *     look for other places where the most-recently-added CPU is mentioned
 *   x file cpu_frequency.h
 *     function get_max_turbo_freq()
 *     go to "switch (model)", and add turbo boosts
 */

#include <sys/types.h>
#include "hwcdrv.h"

static uint64_t num_gpc;    /* number of general purpose counters (e.g. 2-4) */
static uint64_t num_ffc;    /* number fixed function counters (e.g. 3) */
static uint_t total_pmc;    /* num_gpc + num_ffc */

/*
 * Only the lower 32-bits can be written to in the general-purpose
 * counters.  The higher bits are extended from bit 31; all ones if
 * bit 31 is one and all zeros otherwise.
 *
 * The fixed-function counters do not have this restriction.
 */

static const char *ffc_names[] = {
/*
 * While modern Intel processors have fixed-function counters (FFCs),
 * on Linux we access HWCs through the perf_event_open() kernel interface,
 * which does not allow us direct access to the FFCs.
 * Rather, the Linux kernel manages registers opaquely.
 * At best, it allows us extra HW events by off-loading
 * HWCs to FFCs as available.  Often, however, the FFCs
 * are commandeered by other activities like the NMI watchdog.
 * We will omit any explicit reference to them.
 * https://lists.eecs.utk.edu/pipermail/perfapi-devel/2015-February/006895.html
 * See also bug 21315497.
 */
#if 0
	"instr_retired.any",
	"cpu_clk_unhalted.core",
	"cpu_clk_unhalted.ref",
#endif
	NULL
};

#define IMPL_NAME_LEN   100
static char core_impl_name[IMPL_NAME_LEN];

/*
 * Most events require only an event code and a umask.
 * Some also require attributes, cmasks, or MSR programming.
 * Until Sandy Bridge, the number of these other events
 * was small and libcpc just ignored them.
 * With Sandy Bridge, libcpc added for support for these
 * additional events.
 *
 * We use an expanded events_table_t here -- patterned
 * after snb_pcbe_events_table_t in libcpc's
 * usr/src/uts/intel/pcbe/snb_pcbe.h -- for all processors.
 *
 * Correspondingly, we also define ATTR_* macros, but we
 * define them to set bits as they will appear
 * in bits 16-23 of the final eventsel.  Definitions of those
 * bits can be found in "struct ia32_perfevtsel" in libcpc's
 * usr/src/uts/intel/pcbe/intel_pcbe_utils.h .
 *
 * For now, I don't know how to handle msr_offset.
 * So, let's not include events that call for it.
 *
 * For now, don't do anything with ATTR_PEBS other than
 * to note it in tables (starting with Haswell).
 *
 * Solaris tables also have ATTR_PEBS_ONLY.  We cannot
 * use these counters from "collect -h" and so do not
 * include them.
 */
#define ATTR_NONE               0
#define ATTR_EDGE               (1 << 2) /* bit 18 - offset 16 */
#define ATTR_ANY                (1 << 5) /* bit 21 - offset 16 */
#define ATTR_INV                (1 << 7) /* bit 23 - offset 16 */
#define ATTR_PEBS               ATTR_NONE // PEBS not supported
#define ATTR_TSX                ATTR_NONE // TSX MSRs not supported
#undef ATTR_PEBS_ONLY           // PEBS-only event, not supported
#undef ATTR_PEBS_ONLY_LD_LAT    // not supported

struct events_table_t
{
  uint32_t eventselect;
  uint32_t unitmask;
  uint64_t supported_counters;
  const char *name;
  uint8_t cmask;
  uint8_t attrs;
  uint16_t msr_offset;
};

/* Used to describe which counters support an event */
#define C(x)    (1 << (x))
#define C0      C(0)
#define C1      C(1)
#define C2      C(2)
#define C3      C(3)
#define C_ALL   0xFFFFFFFFFFFFFFFF
#define CDEAD   0 /* Counter that is broken  */

/* note that regular events use the original spelling like "inst_retired.any_p" */
#define	ARCH_EVENTS /* NOTE: Order specified in PRM must be maintained! */ \
{ 0x3C, 0x00, C_ALL, "unhalted-core-cycles"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3C, 0x01, C_ALL, "unhalted-reference-cycles"    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC0, 0x00, C_ALL, "instruction-retired"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2E, 0x4F, C_ALL, "llc-reference"    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2E, 0x41, C_ALL, "llc-misses"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x00, C_ALL, "branch-instruction-retired"   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC5, 0x00, C_ALL, "branch-misses-retired"        , 0x0, ATTR_NONE, 0x0 }, \
/* end of #define */

/*
 * FAM6/MOD15:
 *  Xeon 3000, 3200, 5100, 5300, 7300
 *  Core 2 Quad, Extreme, and Duo
 *  Pentium dual-core processors
 * FAM6/MOD23:
 *  Xeon 5200, 5400 series, Intel
 *  Core 2 Quad Q9650.
 */
#define EVENTS_FAM6_MOD23                                               \
{ 0x03, 0x00, C0|C1, "load_block"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x03, 0x02, C0|C1, "load_block.sta"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x03, 0x04, C0|C1, "load_block.std"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x03, 0x08, C0|C1, "load_block.overlap_store"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x03, 0x10, C0|C1, "load_block.until_retire"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x03, 0x20, C0|C1, "load_block.l1d"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x04, 0x00, C0|C1, "store_block"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x04, 0x01, C0|C1, "store_block.drain_cycles" /*spell-diff*/    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x04, 0x02, C0|C1, "store_block.order"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x04, 0x08, C0|C1, "store_block.snoop"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x05, 0x00, C0|C1, "misalign_mem_ref"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x06, 0x00, C0|C1, "segment_reg_loads"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x07, 0x00, C0|C1, "sse_pre_exec"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x07, 0x00, C0|C1, "sse_pre_exec.nta"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x07, 0x01, C0|C1, "sse_pre_exec.l1"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x07, 0x02, C0|C1, "sse_pre_exec.l2"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x07, 0x03, C0|C1, "sse_pre_exec.stores"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x00, C0|C1, "dtlb_misses"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x01, C0|C1, "dtlb_misses.any"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x02, C0|C1, "dtlb_misses.miss_ld"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x04, C0|C1, "dtlb_misses.l0_miss_ld"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x08, C0|C1, "dtlb_misses.miss_st"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x09, 0x00, C0|C1, "memory_disambiguation"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x09, 0x01, C0|C1, "memory_disambiguation.reset"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x09, 0x02, C0|C1, "memory_disambiguation.success"        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0c, 0x00, C0|C1, "page_walks"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0c, 0x01, C0|C1, "page_walks.count"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0c, 0x02, C0|C1, "page_walks.cycles"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x00, C0   , "fp_comp_ops_exe"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x11, 0x00,    C1, "fp_assist"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x12, 0x00,    C1, "mul"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x13, 0x00,    C1, "div"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x14, 0x00, C0   , "cycles_div_busy"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x18, 0x00, C0   , "idle_during_div"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x19, 0x00,    C1, "delayed_bypass"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x19, 0x00,    C1, "delayed_bypass.fp"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x19, 0x01,    C1, "delayed_bypass.simd"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x19, 0x02,    C1, "delayed_bypass.load"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x21, 0x00, C0|C1, "l2_ads"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x23, 0x00, C0|C1, "l2_dbus_busy_rd"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x00, C0|C1, "l2_lines_in"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x25, 0x00, C0|C1, "l2_m_lines_in"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0x00, C0|C1, "l2_lines_out"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x00, C0|C1, "l2_m_lines_out"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x28, 0x00, C0|C1, "l2_ifetch"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x29, 0x00, C0|C1, "l2_ld"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2a, 0x00, C0|C1, "l2_st"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2b, 0x00, C0|C1, "l2_lock"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2e, 0x00, C0|C1, "l2_rqsts"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2e, 0x41, C0|C1, "l2_rqsts.self.demand.i_state"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2e, 0x4f, C0|C1, "l2_rqsts.self.demand.mesi"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x30, 0x00, C0|C1, "l2_reject_busq"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x32, 0x00, C0|C1, "l2_no_req"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3a, 0x00, C0|C1, "eist_trans"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3b, 0xc0, C0|C1, "thermal_trip" /*non-zero umask*/      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3c, 0x00, C0|C1, "cpu_clk_unhalted"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3c, 0x00, C0|C1, "cpu_clk_unhalted.core_p"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3c, 0x01, C0|C1, "cpu_clk_unhalted.bus"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3c, 0x02, C0|C1, "cpu_clk_unhalted.no_other"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x40, 0x00, C0|C1, "l1d_cache_ld"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x41, 0x00, C0|C1, "l1d_cache_st"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x42, 0x00, C0|C1, "l1d_cache_lock"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x42, 0x10, C0|C1, "l1d_cache_lock.duration" /*spelling*/ , 0x0, ATTR_NONE, 0x0 }, \
{ 0x43, 0x00, C0|C1, "l1d_all"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x43, 0x00, C0|C1, "l1d_all_ref"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x43, 0x01, C0|C1, "l1d_all.ref" /*spelling*/             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x43, 0x02, C0|C1, "l1d_all.cache_ref" /*spelling*/       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x45, 0x0f, C0|C1, "l1d_repl" /*non-zero umask*/          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x46, 0x00, C0|C1, "l1d_m_repl"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x47, 0x00, C0|C1, "l1d_m_evict"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x48, 0x00, C0|C1, "l1d_pend_miss"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x00, C0|C1, "l1d_split"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x01, C0|C1, "l1d_split.loads"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x02, C0|C1, "l1d_split.stores"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4b, 0x00, C0|C1, "sse_pre_miss"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4b, 0x00, C0|C1, "sse_pre_miss.nta"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4b, 0x01, C0|C1, "sse_pre_miss.l1"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4b, 0x02, C0|C1, "sse_pre_miss.l2"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4c, 0x00, C0|C1, "load_hit_pre"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4e, 0x00, C0|C1, "l1d_prefetch"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4e, 0x10, C0|C1, "l1d_prefetch.requests"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x00, C0|C1, "bus_request_outstanding"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x61, 0x00, C0|C1, "bus_bnr_drv"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x62, 0x00, C0|C1, "bus_drdy_clocks"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x63, 0x00, C0|C1, "bus_lock_clocks"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x64, 0x00, C0|C1, "bus_data_rcv"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x65, 0x00, C0|C1, "bus_trans_brd"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x66, 0x00, C0|C1, "bus_trans_rfo"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x67, 0x00, C0|C1, "bus_trans_wb"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x68, 0x00, C0|C1, "bus_trans_ifetch"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x69, 0x00, C0|C1, "bus_trans_inval"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x6a, 0x00, C0|C1, "bus_trans_pwr"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x6b, 0x00, C0|C1, "bus_trans_p"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x6c, 0x00, C0|C1, "bus_trans_io"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x6d, 0x00, C0|C1, "bus_trans_def"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x6e, 0x00, C0|C1, "bus_trans_burst"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x6f, 0x00, C0|C1, "bus_trans_mem"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x70, 0x00, C0|C1, "bus_trans_any"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x77, 0x00, C0|C1, "ext_snoop"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x78, 0x00, C0|C1, "cmp_snoop"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x7a, 0x00, C0|C1, "bus_hit_drv"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x7b, 0x00, C0|C1, "bus_hitm_drv"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x7d, 0x00, C0|C1, "busq_empty"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x7e, 0x00, C0|C1, "snoop_stall_drv"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x7f, 0x00, C0|C1, "bus_io_wait"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x80, 0x00, C0|C1, "l1i_reads"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x81, 0x00, C0|C1, "l1i_misses"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x82, 0x00, C0|C1, "itlb"                                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x82, 0x02, C0|C1, "itlb.small_miss"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x82, 0x10, C0|C1, "itlb.large_miss"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x82, 0x12, C0|C1, "itlb.misses"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x82, 0x40, C0|C1, "itlb.flush"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x83, 0x00, C0|C1, "inst_queue"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x83, 0x02, C0|C1, "inst_queue.full"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x86, 0x00, C0|C1, "cycles_l1i_mem_stalled"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x87, 0x00, C0|C1, "ild_stall"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x00, C0|C1, "br_inst_exec"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x00, C0|C1, "br_missp_exec"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x8a, 0x00, C0|C1, "br_bac_missp_exec"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x8b, 0x00, C0|C1, "br_cnd_exec"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x8c, 0x00, C0|C1, "br_cnd_missp_exec"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x8d, 0x00, C0|C1, "br_ind_exec"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x8e, 0x00, C0|C1, "br_ind_missp_exec"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x8f, 0x00, C0|C1, "br_ret_exec"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x90, 0x00, C0|C1, "br_ret_missp_exec"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x91, 0x00, C0|C1, "br_ret_bac_missp_exec"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x92, 0x00, C0|C1, "br_call_exec"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x93, 0x00, C0|C1, "br_call_missp_exec"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x94, 0x00, C0|C1, "br_ind_call_exec"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x97, 0x00, C0|C1, "br_tkn_bubble_1"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x98, 0x00, C0|C1, "br_tkn_bubble_2"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xa0, 0x00, C0|C1, "rs_uops_dispatched"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xa1, 0x00, C0   , "rs_uops_dispatched_port"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xa1, 0x01, C0   , "rs_uops_dispatched_port.0" /*spelling*/     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xa1, 0x02, C0   , "rs_uops_dispatched_port.1" /*spelling*/     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xa1, 0x04, C0   , "rs_uops_dispatched_port.2" /*spelling*/     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xa1, 0x08, C0   , "rs_uops_dispatched_port.3" /*spelling*/     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xa1, 0x10, C0   , "rs_uops_dispatched_port.4" /*spelling*/     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xa1, 0x20, C0   , "rs_uops_dispatched_port.5" /*spelling*/     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xaa, 0x00, C0|C1, "macro_insts"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xaa, 0x01, C0|C1, "macro_insts.decoded"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xaa, 0x08, C0|C1, "macro_insts.cisc_decoded"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xab, 0x00, C0|C1, "esp"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xab, 0x01, C0|C1, "esp.synch"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xab, 0x02, C0|C1, "esp.additions"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb0, 0x00, C0|C1, "simd_uops_exec"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb1, 0x00, C0|C1, "simd_sat_uop_exec"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb3, 0x00, C0|C1, "simd_uop_type_exec"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb3, 0x01, C0|C1, "simd_uop_type_exec.mul"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb3, 0x02, C0|C1, "simd_uop_type_exec.shift"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb3, 0x04, C0|C1, "simd_uop_type_exec.pack"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb3, 0x08, C0|C1, "simd_uop_type_exec.unpack"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb3, 0x10, C0|C1, "simd_uop_type_exec.logical"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb3, 0x20, C0|C1, "simd_uop_type_exec.arithmetic"        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc0, 0x00, C0|C1, "inst_retired"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc0, 0x00, C0|C1, "inst_retired.any_p"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc0, 0x01, C0|C1, "inst_retired.loads"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc0, 0x02, C0|C1, "inst_retired.stores"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc0, 0x04, C0|C1, "inst_retired.other"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc0, 0x08, C0|C1, "inst_retired.vm_h"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc1, 0x00, C0|C1, "x87_ops_retired"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc1, 0x01, C0|C1, "x87_ops_retired.fxch"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc1, 0xfe, C0|C1, "x87_ops_retired.any"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc2, 0x00, C0|C1, "uops_retired"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc2, 0x01, C0|C1, "uops_retired.ld_ind_br"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc2, 0x02, C0|C1, "uops_retired.std_sta"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc2, 0x04, C0|C1, "uops_retired.macro_fusion"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc2, 0x07, C0|C1, "uops_retired.fused"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc2, 0x08, C0|C1, "uops_retired.non_fused"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc2, 0x0f, C0|C1, "uops_retired.any"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc3, 0x00, C0|C1, "machine_nukes"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc3, 0x01, C0|C1, "machine_nukes.smc"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc3, 0x04, C0|C1, "machine_nukes.mem_order"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc4, 0x00, C0|C1, "br_inst_retired"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc4, 0x00, C0|C1, "br_inst_retired.any"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc4, 0x01, C0|C1, "br_inst_retired.pred_not_taken"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc4, 0x02, C0|C1, "br_inst_retired.mispred_not_taken"    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc4, 0x04, C0|C1, "br_inst_retired.pred_taken"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc4, 0x08, C0|C1, "br_inst_retired.mispred_taken"        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc4, 0x0c, C0|C1, "br_inst_retired.taken"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc5, 0x00, C0|C1, "br_inst_retired_mispred"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc5, 0x00, C0|C1, "br_inst_retired.mispred" /*alt-spelling*/   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc6, 0x00, C0|C1, "cycles_int"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc6, 0x01, C0|C1, "cycles_int.masked" /*spelling*/       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc6, 0x02, C0|C1, "cycles_int.pending_and_masked" /*spelling*/ , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc7, 0x00, C0|C1, "simd_inst_retired"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc7, 0x01, C0|C1, "simd_inst_retired.packed_single"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc7, 0x02, C0|C1, "simd_inst_retired.scalar_single"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc7, 0x04, C0|C1, "simd_inst_retired.packed_double"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc7, 0x08, C0|C1, "simd_inst_retired.scalar_double"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc7, 0x10, C0|C1, "simd_inst_retired.vector"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc7, 0x1f, C0|C1, "simd_inst_retired.any"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc8, 0x00, C0|C1, "hw_int_rcv"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc9, 0x00, C0|C1, "itlb_miss_retired"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xca, 0x00, C0|C1, "simd_comp_inst_retired"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xca, 0x01, C0|C1, "simd_comp_inst_retired.packed_single" , 0x0, ATTR_NONE, 0x0 }, \
{ 0xca, 0x02, C0|C1, "simd_comp_inst_retired.scalar_single" , 0x0, ATTR_NONE, 0x0 }, \
{ 0xca, 0x04, C0|C1, "simd_comp_inst_retired.packed_double" , 0x0, ATTR_NONE, 0x0 }, \
{ 0xca, 0x08, C0|C1, "simd_comp_inst_retired.scalar_double" , 0x0, ATTR_NONE, 0x0 }, \
{ 0xcb, 0x00, C0   , "mem_load_retired"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xcb, 0x01, C0   , "mem_load_retired.l1d_miss"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xcb, 0x02, C0   , "mem_load_retired.l1d_line_miss"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xcb, 0x04, C0   , "mem_load_retired.l2_miss"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xcb, 0x08, C0   , "mem_load_retired.l2_line_miss"        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xcb, 0x10, C0   , "mem_load_retired.dtlb_miss"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xcc, 0x00, C0|C1, "fp_mmx_trans"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xcc, 0x01, C0|C1, "fp_mmx_trans.to_mmx" /*spelling*/     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xcc, 0x02, C0|C1, "fp_mmx_trans.to_fp" /*spelling*/      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xcd, 0x00, C0|C1, "simd_assist"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xce, 0x00, C0|C1, "simd_instr_retired"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xcf, 0x00, C0|C1, "simd_sat_instr_retired"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xd2, 0x00, C0|C1, "rat_stalls"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xd2, 0x01, C0|C1, "rat_stalls.rob_read_port"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xd2, 0x02, C0|C1, "rat_stalls.partial_cycles"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xd2, 0x04, C0|C1, "rat_stalls.flags"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xd2, 0x08, C0|C1, "rat_stalls.fpsw"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xd2, 0x0f, C0|C1, "rat_stalls.any"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xd2, 0x10, C0|C1, "rat_stalls.other_serialization_stalls", 0x0, ATTR_NONE, 0x0 }, \
{ 0xd4, 0x00, C0|C1, "seg_rename_stalls"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xd4, 0x01, C0|C1, "seg_rename_stalls.es"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xd4, 0x02, C0|C1, "seg_rename_stalls.ds"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xd4, 0x04, C0|C1, "seg_rename_stalls.fs"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xd4, 0x08, C0|C1, "seg_rename_stalls.gs"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xd4, 0x0f, C0|C1, "seg_rename_stalls.any"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xd5, 0x00, C0|C1, "seg_reg_renames"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xd5, 0x01, C0|C1, "seg_reg_renames.es"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xd5, 0x02, C0|C1, "seg_reg_renames.ds"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xd5, 0x04, C0|C1, "seg_reg_renames.fs"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xd5, 0x08, C0|C1, "seg_reg_renames.gs"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xd5, 0x0f, C0|C1, "seg_reg_renames.any"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xdc, 0x00, C0|C1, "resource_stalls"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xdc, 0x01, C0|C1, "resource_stalls.rob_full"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xdc, 0x02, C0|C1, "resource_stalls.rs_full"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xdc, 0x04, C0|C1, "resource_stalls.ld_st"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xdc, 0x08, C0|C1, "resource_stalls.fpcw"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xdc, 0x10, C0|C1, "resource_stalls.br_miss_clear"        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xdc, 0x1f, C0|C1, "resource_stalls.any"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xe0, 0x00, C0|C1, "br_inst_decoded"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xe4, 0x00, C0|C1, "bogus_br"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xe6, 0x00, C0|C1, "baclears"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xf0, 0x00, C0|C1, "pref_rqsts_up"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xf8, 0x00, C0|C1, "pref_rqsts_dn"                        , 0x0, ATTR_NONE, 0x0 }, \
/* end of #define */

/* FAM6 MOD28: Intel Atom processor */
#define EVENTS_FAM6_MOD28                                               \
{ 0x02, 0x81, C0|C1, "store_forwards.good"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x06, 0x00, C0|C1, "segment_reg_loads.any"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x07, 0x01, C0|C1, "prefetch.prefetcht0"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x07, 0x06, C0|C1, "prefetch.sw_l2"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x07, 0x08, C0|C1, "prefetch.prefetchnta"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x05, C0|C1, "data_tlb_misses.dtlb_miss_ld"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x06, C0|C1, "data_tlb_misses.dtlb_miss_st"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x07, C0|C1, "data_tlb_misses.dtlb_miss"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x09, C0|C1, "data_tlb_misses.l0_dtlb_miss_ld"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0c, 0x03, C0|C1, "page_walks.cycles"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x01, C0|C1, "x87_comp_ops_exe.any.s"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x81, C0|C1, "x87_comp_ops_exe.any.ar"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x11, 0x01, C0|C1, "fp_assist"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x11, 0x81, C0|C1, "fp_assist.ar"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x12, 0x01, C0|C1, "mul.s"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x12, 0x81, C0|C1, "mul.ar"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x13, 0x01, C0|C1, "div.s"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x13, 0x81, C0|C1, "div.ar"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x14, 0x01, C0|C1, "cycles_div_busy"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x21, 0x00, C0|C1, "l2_ads"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x22, 0x00, C0|C1, "l2_dbus_busy"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x00, C0|C1, "l2_lines_in"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x25, 0x00, C0|C1, "l2_m_lines_in"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0x00, C0|C1, "l2_lines_out"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x00, C0|C1, "l2_m_lines_out"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x28, 0x00, C0|C1, "l2_ifetch"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x29, 0x00, C0|C1, "l2_ld"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2a, 0x00, C0|C1, "l2_st"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2b, 0x00, C0|C1, "l2_lock"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2e, 0x00, C0|C1, "l2_rqsts"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2e, 0x41, C0|C1, "l2_rqsts.self.demand.i_state"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2e, 0x4f, C0|C1, "l2_rqsts.self.demand.mesi"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x30, 0x00, C0|C1, "l2_reject_busq"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x32, 0x00, C0|C1, "l2_no_req"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3a, 0x00, C0|C1, "eist_trans"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3b, 0xc0, C0|C1, "thermal_trip"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3c, 0x00, C0|C1, "cpu_clk_unhalted.core_p"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3c, 0x01, C0|C1, "cpu_clk_unhalted.bus"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3c, 0x02, C0|C1, "cpu_clk_unhalted.no_other"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x40, 0x21, C0|C1, "l1d_cache.ld"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x40, 0x22, C0|C1, "l1d_cache.st"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x00, C0|C1, "bus_request_outstanding"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x61, 0x00, C0|C1, "bus_bnr_drv"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x62, 0x00, C0|C1, "bus_drdy_clocks"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x63, 0x00, C0|C1, "bus_lock_clocks"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x64, 0x00, C0|C1, "bus_data_rcv"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x65, 0x00, C0|C1, "bus_trans_brd"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x66, 0x00, C0|C1, "bus_trans_rfo"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x67, 0x00, C0|C1, "bus_trans_wb"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x68, 0x00, C0|C1, "bus_trans_ifetch"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x69, 0x00, C0|C1, "bus_trans_inval"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x6a, 0x00, C0|C1, "bus_trans_pwr"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x6b, 0x00, C0|C1, "bus_trans_p"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x6c, 0x00, C0|C1, "bus_trans_io"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x6d, 0x00, C0|C1, "bus_trans_def"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x6e, 0x00, C0|C1, "bus_trans_burst"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x6f, 0x00, C0|C1, "bus_trans_mem"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x70, 0x00, C0|C1, "bus_trans_any"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x77, 0x00, C0|C1, "ext_snoop"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x7a, 0x00, C0|C1, "bus_hit_drv"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x7b, 0x00, C0|C1, "bus_hitm_drv"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x7d, 0x00, C0|C1, "busq_empty"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x7e, 0x00, C0|C1, "snoop_stall_drv"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x7f, 0x00, C0|C1, "bus_io_wait"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x80, 0x02, C0|C1, "icache.misses"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x80, 0x03, C0|C1, "icache.accesses"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x82, 0x02, C0|C1, "itlb.misses"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x82, 0x04, C0|C1, "itlb.flush"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xaa, 0x02, C0|C1, "macro_insts.cisc_decoded"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xaa, 0x03, C0|C1, "macro_insts.all_decoded"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb0, 0x00, C0|C1, "simd_uops_exec.s"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb0, 0x80, C0|C1, "simd_uops_exec.ar"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb1, 0x00, C0|C1, "simd_sat_uop_exec.s"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb1, 0x80, C0|C1, "simd_sat_uop_exec.ar"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb3, 0x01, C0|C1, "simd_uop_type_exec.mul.s"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb3, 0x02, C0|C1, "simd_uop_type_exec.shift.s"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb3, 0x04, C0|C1, "simd_uop_type_exec.pack.s"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb3, 0x08, C0|C1, "simd_uop_type_exec.unpack.s"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb3, 0x10, C0|C1, "simd_uop_type_exec.logical.s"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb3, 0x20, C0|C1, "simd_uop_type_exec.arithmetic.s"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb3, 0x81, C0|C1, "simd_uop_type_exec.mul.ar"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb3, 0x82, C0|C1, "simd_uop_type_exec.shift.ar"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb3, 0x84, C0|C1, "simd_uop_type_exec.pack.ar"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb3, 0x88, C0|C1, "simd_uop_type_exec.unpack.ar"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb3, 0x90, C0|C1, "simd_uop_type_exec.logical.ar"        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xb3, 0xa0, C0|C1, "simd_uop_type_exec.arithmetic.ar"     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc0, 0x00, C0|C1, "inst_retired.any_p"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc2, 0x10, C0|C1, "uops_retired.any"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc3, 0x01, C0|C1, "machine_clears.smc"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc4, 0x00, C0|C1, "br_inst_retired.any"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc4, 0x01, C0|C1, "br_inst_retired.pred_not_taken"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc4, 0x02, C0|C1, "br_inst_retired.mispred_not_taken"    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc4, 0x04, C0|C1, "br_inst_retired.pred_taken"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc4, 0x08, C0|C1, "br_inst_retired.mispred_taken"        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc4, 0x0a, C0|C1, "br_inst_retired.mispred"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc4, 0x0c, C0|C1, "br_inst_retired.taken"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc4, 0x0f, C0|C1, "br_inst_retired.any1"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc5, 0x00, C0|C1, "br_inst_retired.mispred"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc6, 0x01, C0|C1, "cycles_int_masked.cycles_int_masked"  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc6, 0x02, C0|C1, "cycles_int_masked.cycles_int_pending_and_masked" , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc7, 0x01, C0|C1, "simd_inst_retired.packed_single"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc7, 0x02, C0|C1, "simd_inst_retired.scalar_single"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc7, 0x04, C0|C1, "simd_inst_retired.packed_double"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc7, 0x08, C0|C1, "simd_inst_retired.scalar_double"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc7, 0x10, C0|C1, "simd_inst_retired.vector"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc7, 0x1f, C0|C1, "simd_inst_retired.any"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc8, 0x00, C0|C1, "hw_int_rcv"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xca, 0x01, C0|C1, "simd_comp_inst_retired.packed_single" , 0x0, ATTR_NONE, 0x0 }, \
{ 0xca, 0x02, C0|C1, "simd_comp_inst_retired.scalar_single" , 0x0, ATTR_NONE, 0x0 }, \
{ 0xca, 0x04, C0|C1, "simd_comp_inst_retired.packed_double" , 0x0, ATTR_NONE, 0x0 }, \
{ 0xca, 0x08, C0|C1, "simd_comp_inst_retired.scalar_double" , 0x0, ATTR_NONE, 0x0 }, \
{ 0xcb, 0x01, C0|C1, "mem_load_retired.l2_hit"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xcb, 0x02, C0|C1, "mem_load_retired.l2_miss"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xcb, 0x04, C0|C1, "mem_load_retired.dtlb_miss"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xcd, 0x00, C0|C1, "simd_assist"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xce, 0x00, C0|C1, "simd_instr_retired"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xcf, 0x00, C0|C1, "simd_sat_instr_retired"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xe0, 0x01, C0|C1, "br_inst_decoded"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xe4, 0x01, C0|C1, "bogus_br"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xe6, 0x01, C0|C1, "baclears.any"                         , 0x0, ATTR_NONE, 0x0 }, \
/* end of #define */

/* Intel Core i7 (Nehalem) Processor */
/*
 * The Nehalem tables are basically from Bug 16457009
 *   libcpc counter names should be based on public Intel documentation -- Nehalem
 * and those tables are basically from the
 * Intel SDM, January 2013, Section 19.5, Table 19-11.
 * We omit the Table 19-12 uncore events.
 *
 * Note that the table below includes some events from
 * the Intel SDM that require cmask or attr settings.
 * These events are not in libcpc, which did not include
 * events requiring cmask or attr until Sandy Bridge.
 */

#define EVENTS_FAM6_MOD26                                               \
{ 0x04, 0x07, C0|C1|C2|C3, "sb_drain.any"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x06, 0x04, C0|C1|C2|C3, "store_blocks.at_ret"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x06, 0x08, C0|C1|C2|C3, "store_blocks.l1d_block"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x07, 0x01, C0|C1|C2|C3, "partial_address_alias"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x01, C0|C1|C2|C3, "dtlb_load_misses.any"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x02, C0|C1|C2|C3, "dtlb_load_misses.walk_completed"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x10, C0|C1|C2|C3, "dtlb_load_misses.stlb_hit"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x20, C0|C1|C2|C3, "dtlb_load_misses.pde_miss"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x80, C0|C1|C2|C3, "dtlb_load_misses.large_walk_completed", 0x0, ATTR_NONE, 0x0 }, \
{ 0x0B, 0x01, C0|C1|C2|C3, "mem_inst_retired.loads"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0B, 0x02, C0|C1|C2|C3, "mem_inst_retired.stores"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0B, 0x10, C0|C1|C2|C3, "mem_inst_retired.latency_above_threshold"   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0C, 0x01, C0|C1|C2|C3, "mem_store_retired.dtlb_miss"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0E, 0x01, C0|C1|C2|C3, "uops_issued.any"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0E, 0x01, C0|C1|C2|C3, "uops_issued.stalled_cycles"           , 0x1, ATTR_INV , 0x0 }, \
{ 0x0E, 0x02, C0|C1|C2|C3, "uops_issued.fused"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0F, 0x02, C0|C1|C2|C3, "mem_uncore_retired.other_core_l2_hitm", 0x0, ATTR_NONE, 0x0 }, \
{ 0x0F, 0x08, C0|C1|C2|C3, "mem_uncore_retired.remote_cache_local_home_hit", 0x0, ATTR_NONE, 0x0 }, \
{ 0x0F, 0x10, C0|C1|C2|C3, "mem_uncore_retired.remote_dram"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0F, 0x20, C0|C1|C2|C3, "mem_uncore_retired.local_dram"        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x01, C0|C1|C2|C3, "fp_comp_ops_exe.x87"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x02, C0|C1|C2|C3, "fp_comp_ops_exe.mmx"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x04, C0|C1|C2|C3, "fp_comp_ops_exe.sse_fp"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x08, C0|C1|C2|C3, "fp_comp_ops_exe.sse2_integer"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x10, C0|C1|C2|C3, "fp_comp_ops_exe.sse_fp_packed"        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x20, C0|C1|C2|C3, "fp_comp_ops_exe.sse_fp_scalar"        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x40, C0|C1|C2|C3, "fp_comp_ops_exe.sse_single_precision" , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x80, C0|C1|C2|C3, "fp_comp_ops_exe.sse_double_precision" , 0x0, ATTR_NONE, 0x0 }, \
{ 0x12, 0x01, C0|C1|C2|C3, "simd_int_128.packed_mpy"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x12, 0x02, C0|C1|C2|C3, "simd_int_128.packed_shift"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x12, 0x04, C0|C1|C2|C3, "simd_int_128.pack"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x12, 0x08, C0|C1|C2|C3, "simd_int_128.unpack"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x12, 0x10, C0|C1|C2|C3, "simd_int_128.packed_logical"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x12, 0x20, C0|C1|C2|C3, "simd_int_128.packed_arith"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x12, 0x40, C0|C1|C2|C3, "simd_int_128.shuffle_move"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x13, 0x01, C0|C1|C2|C3, "load_dispatch.rs"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x13, 0x02, C0|C1|C2|C3, "load_dispatch.rs_delayed"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x13, 0x04, C0|C1|C2|C3, "load_dispatch.mob"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x13, 0x07, C0|C1|C2|C3, "load_dispatch.any"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x14, 0x01, C0|C1|C2|C3, "arith.cycles_div_busy"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x14, 0x01, C0|C1|C2|C3, "arith.fpu_div"                        , 0x1, ATTR_EDGE | ATTR_INV, 0x0 }, \
{ 0x14, 0x02, C0|C1|C2|C3, "arith.mul"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x17, 0x01, C0|C1|C2|C3, "inst_queue_writes"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x18, 0x01, C0|C1|C2|C3, "inst_decoded.dec0"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x19, 0x01, C0|C1|C2|C3, "two_uop_insts_decoded"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x1E, 0x01, C0|C1|C2|C3, "inst_queue_write_cycles"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x20, 0x01, C0|C1|C2|C3, "lsd_overflow"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x01, C0|C1|C2|C3, "l2_rqsts.ld_hit"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x02, C0|C1|C2|C3, "l2_rqsts.ld_miss"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x03, C0|C1|C2|C3, "l2_rqsts.loads"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x04, C0|C1|C2|C3, "l2_rqsts.rfo_hit"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x08, C0|C1|C2|C3, "l2_rqsts.rfo_miss"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x0C, C0|C1|C2|C3, "l2_rqsts.rfos"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x10, C0|C1|C2|C3, "l2_rqsts.ifetch_hit"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x20, C0|C1|C2|C3, "l2_rqsts.ifetch_miss"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x30, C0|C1|C2|C3, "l2_rqsts.ifetches"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x40, C0|C1|C2|C3, "l2_rqsts.prefetch_hit"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x80, C0|C1|C2|C3, "l2_rqsts.prefetch_miss"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xAA, C0|C1|C2|C3, "l2_rqsts.miss"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xC0, C0|C1|C2|C3, "l2_rqsts.prefetches"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xFF, C0|C1|C2|C3, "l2_rqsts.references"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0x01, C0|C1|C2|C3, "l2_data_rqsts.demand.i_state"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0x02, C0|C1|C2|C3, "l2_data_rqsts.demand.s_state"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0x04, C0|C1|C2|C3, "l2_data_rqsts.demand.e_state"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0x08, C0|C1|C2|C3, "l2_data_rqsts.demand.m_state"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0x0F, C0|C1|C2|C3, "l2_data_rqsts.demand.mesi"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0x10, C0|C1|C2|C3, "l2_data_rqsts.prefetch.i_state"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0x20, C0|C1|C2|C3, "l2_data_rqsts.prefetch.s_state"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0x40, C0|C1|C2|C3, "l2_data_rqsts.prefetch.e_state"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0x80, C0|C1|C2|C3, "l2_data_rqsts.prefetch.m_state"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0xF0, C0|C1|C2|C3, "l2_data_rqsts.prefetch.mesi"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0xFF, C0|C1|C2|C3, "l2_data_rqsts.any"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x01, C0|C1|C2|C3, "l2_write.rfo.i_state"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x02, C0|C1|C2|C3, "l2_write.rfo.s_state"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x08, C0|C1|C2|C3, "l2_write.rfo.m_state"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x0E, C0|C1|C2|C3, "l2_write.rfo.hit"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x0F, C0|C1|C2|C3, "l2_write.rfo.mesi"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x10, C0|C1|C2|C3, "l2_write.lock.i_state"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x20, C0|C1|C2|C3, "l2_write.lock.s_state"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x40, C0|C1|C2|C3, "l2_write.lock.e_state"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x80, C0|C1|C2|C3, "l2_write.lock.m_state"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0xE0, C0|C1|C2|C3, "l2_write.lock.hit"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0xF0, C0|C1|C2|C3, "l2_write.lock.mesi"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x28, 0x01, C0|C1|C2|C3, "l1d_wb_l2.i_state"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x28, 0x02, C0|C1|C2|C3, "l1d_wb_l2.s_state"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x28, 0x04, C0|C1|C2|C3, "l1d_wb_l2.e_state"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x28, 0x08, C0|C1|C2|C3, "l1d_wb_l2.m_state"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x28, 0x0F, C0|C1|C2|C3, "l1d_wb_l2.mesi"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2E, 0x41, C0|C1|C2|C3, "l3_lat_cache.miss"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2E, 0x4F, C0|C1|C2|C3, "l3_lat_cache.reference"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3C, 0x00, C0|C1|C2|C3, "cpu_clk_unhalted.thread_p"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3C, 0x01, C0|C1|C2|C3, "cpu_clk_unhalted.ref_p"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x40, 0x01, C0|C1      , "l1d_cache_ld.i_state"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x40, 0x02, C0|C1      , "l1d_cache_ld.s_state"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x40, 0x04, C0|C1      , "l1d_cache_ld.e_state"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x40, 0x08, C0|C1      , "l1d_cache_ld.m_state"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x40, 0x0F, C0|C1      , "l1d_cache_ld.mesi"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x41, 0x02, C0|C1      , "l1d_cache_st.s_state"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x41, 0x04, C0|C1      , "l1d_cache_st.e_state"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x41, 0x08, C0|C1      , "l1d_cache_st.m_state"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x42, 0x01, C0|C1      , "l1d_cache_lock.hit"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x42, 0x02, C0|C1      , "l1d_cache_lock.s_state"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x42, 0x04, C0|C1      , "l1d_cache_lock.e_state"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x42, 0x08, C0|C1      , "l1d_cache_lock.m_state"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x43, 0x01, C0|C1      , "l1d_all_ref.any"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x43, 0x02, C0|C1      , "l1d_all_ref.cacheable"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x01, C0|C1|C2|C3, "dtlb_misses.any"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x02, C0|C1|C2|C3, "dtlb_misses.walk_completed"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x10, C0|C1|C2|C3, "dtlb_misses.stlb_hit"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x20, C0|C1|C2|C3, "dtlb_misses.pde_miss"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x80, C0|C1|C2|C3, "dtlb_misses.large_walk_completed"     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4C, 0x01, C0|C1|C2|C3, "load_hit_pre"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4E, 0x01, C0|C1|C2|C3, "l1d_prefetch.requests"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4E, 0x02, C0|C1|C2|C3, "l1d_prefetch.miss"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4E, 0x04, C0|C1|C2|C3, "l1d_prefetch.triggers"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x51, 0x01, C0|C1      , "l1d.repl"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x51, 0x02, C0|C1      , "l1d.m_repl"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x51, 0x04, C0|C1      , "l1d.m_evict"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x51, 0x08, C0|C1      , "l1d.m_snoop_evict"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x52, 0x01, C0|C1|C2|C3, "l1d_cache_prefetch_lock_fb_hit"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x53, 0x01, C0|C1|C2|C3, "l1d_cache_lock_fb_hit"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x63, 0x01, C0|C1      , "cache_lock_cycles.l1d_l2"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x63, 0x02, C0|C1      , "cache_lock_cycles.l1d"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x6C, 0x01, C0|C1|C2|C3, "io_transactions"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x80, 0x01, C0|C1|C2|C3, "l1i.hits"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x80, 0x02, C0|C1|C2|C3, "l1i.misses"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x80, 0x03, C0|C1|C2|C3, "l1i.reads"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x80, 0x04, C0|C1|C2|C3, "l1i.cycles_stalled"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x82, 0x01, C0|C1|C2|C3, "large_itlb.hit"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x01, C0|C1|C2|C3, "itlb_misses.any"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x02, C0|C1|C2|C3, "itlb_misses.walk_completed"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x87, 0x01, C0|C1|C2|C3, "ild_stall.lcp"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x87, 0x02, C0|C1|C2|C3, "ild_stall.mru"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x87, 0x04, C0|C1|C2|C3, "ild_stall.iq_full"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x87, 0x08, C0|C1|C2|C3, "ild_stall.regen"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x87, 0x0F, C0|C1|C2|C3, "ild_stall.any"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x01, C0|C1|C2|C3, "br_inst_exec.cond"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x02, C0|C1|C2|C3, "br_inst_exec.direct"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x04, C0|C1|C2|C3, "br_inst_exec.indirect_non_call"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x07, C0|C1|C2|C3, "br_inst_exec.non_calls"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x08, C0|C1|C2|C3, "br_inst_exec.return_near"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x10, C0|C1|C2|C3, "br_inst_exec.direct_near_call"        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x20, C0|C1|C2|C3, "br_inst_exec.indirect_near_call"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x30, C0|C1|C2|C3, "br_inst_exec.near_calls"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x40, C0|C1|C2|C3, "br_inst_exec.taken"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x7F, C0|C1|C2|C3, "br_inst_exec.any"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x01, C0|C1|C2|C3, "br_misp_exec.cond"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x02, C0|C1|C2|C3, "br_misp_exec.direct"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x04, C0|C1|C2|C3, "br_misp_exec.indirect_non_call"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x07, C0|C1|C2|C3, "br_misp_exec.non_calls"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x08, C0|C1|C2|C3, "br_misp_exec.return_near"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x10, C0|C1|C2|C3, "br_misp_exec.direct_near_call"        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x20, C0|C1|C2|C3, "br_misp_exec.indirect_near_call"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x30, C0|C1|C2|C3, "br_misp_exec.near_calls"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x40, C0|C1|C2|C3, "br_misp_exec.taken"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x7F, C0|C1|C2|C3, "br_misp_exec.any"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x01, C0|C1|C2|C3, "resource_stalls.any"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x02, C0|C1|C2|C3, "resource_stalls.load"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x04, C0|C1|C2|C3, "resource_stalls.rs_full"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x08, C0|C1|C2|C3, "resource_stalls.store"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x10, C0|C1|C2|C3, "resource_stalls.rob_full"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x20, C0|C1|C2|C3, "resource_stalls.fpcw"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x40, C0|C1|C2|C3, "resource_stalls.mxcsr"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x80, C0|C1|C2|C3, "resource_stalls.other"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA6, 0x01, C0|C1|C2|C3, "macro_insts.fusions_decoded"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA7, 0x01, C0|C1|C2|C3, "baclear_force_iq"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA8, 0x01, C0|C1|C2|C3, "lsd.uops"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA8, 0x01, C0|C1|C2|C3, "lsd.cycles"                           , 0x1, ATTR_INV , 0x0 }, \
{ 0xAE, 0x01, C0|C1|C2|C3, "itlb_flush"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x40, C0|C1|C2|C3, "offcore_requests.l1d_writeback"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x01, C0|C1|C2|C3, "uops_executed.port0"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x02, C0|C1|C2|C3, "uops_executed.port1"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x04, C0|C1|C2|C3, "uops_executed.port2_core"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x08, C0|C1|C2|C3, "uops_executed.port3_core"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x10, C0|C1|C2|C3, "uops_executed.port4_core"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x1F, C0|C1|C2|C3, "uops_executed.core_active_cycles_no_port5"  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x20, C0|C1|C2|C3, "uops_executed.port5"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x3F, C0|C1|C2|C3, "uops_executed.core_active_cycles"     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x40, C0|C1|C2|C3, "uops_executed.port015"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x40, C0|C1|C2|C3, "uops_executed.port015_stall_cycles"   , 0x1, ATTR_INV , 0x0 }, \
{ 0xB1, 0x80, C0|C1|C2|C3, "uops_executed.port234"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB2, 0x01, C0|C1|C2|C3, "offcore_requests_sq_full"             , 0x0, ATTR_NONE, 0x0 }, \
/* { 0xB7, 0x01, C0|C1|C2|C3, "off_core_response_0"                  , 0x0, ATTR_NONE, 0x1A6 }, ignore events that require msr_offset */ \
{ 0xB8, 0x01, C0|C1|C2|C3, "snoop_response.hit"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB8, 0x02, C0|C1|C2|C3, "snoop_response.hite"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB8, 0x04, C0|C1|C2|C3, "snoop_response.hitm"                  , 0x0, ATTR_NONE, 0x0 }, \
/* { 0xBB, 0x01, C0|C1|C2|C3, "off_core_response_1"                  , 0x0, ATTR_NONE, 0x1A7 }, ignore events that require msr_offset */ \
{ 0xC0, 0x00, C0|C1|C2|C3, "inst_retired.any_p"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC0, 0x02, C0|C1|C2|C3, "inst_retired.x87"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC0, 0x04, C0|C1|C2|C3, "inst_retired.mmx"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC2, 0x01, C0|C1|C2|C3, "uops_retired.any"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC2, 0x01, C0|C1|C2|C3, "uops_retired.active_cycles"           , 0x1, ATTR_NONE, 0x0 }, \
{ 0xC2, 0x01, C0|C1|C2|C3, "uops_retired.stall_cycles"            , 0x1, ATTR_INV , 0x0 }, \
{ 0xC2, 0x02, C0|C1|C2|C3, "uops_retired.retire_slots"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC2, 0x04, C0|C1|C2|C3, "uops_retired.macro_fused"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC3, 0x01, C0|C1|C2|C3, "machine_clears.cycles"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC3, 0x02, C0|C1|C2|C3, "machine_clears.mem_order"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC3, 0x04, C0|C1|C2|C3, "machine_clears.smc"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x00, C0|C1|C2|C3, "br_inst_retired.all_branches"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x01, C0|C1|C2|C3, "br_inst_retired.conditional"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x02, C0|C1|C2|C3, "br_inst_retired.near_call"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC5, 0x00, C0|C1|C2|C3, "br_misp_retired.all_branches"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC5, 0x02, C0|C1|C2|C3, "br_misp_retired.near_call"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC7, 0x01, C0|C1|C2|C3, "ssex_uops_retired.packed_single"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC7, 0x02, C0|C1|C2|C3, "ssex_uops_retired.scalar_single"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC7, 0x04, C0|C1|C2|C3, "ssex_uops_retired.packed_double"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC7, 0x08, C0|C1|C2|C3, "ssex_uops_retired.scalar_double"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC7, 0x10, C0|C1|C2|C3, "ssex_uops_retired.vector_integer"     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC8, 0x20, C0|C1|C2|C3, "itlb_miss_retired"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCB, 0x01, C0|C1|C2|C3, "mem_load_retired.l1d_hit"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCB, 0x02, C0|C1|C2|C3, "mem_load_retired.l2_hit"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCB, 0x04, C0|C1|C2|C3, "mem_load_retired.llc_unshared_hit"    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCB, 0x08, C0|C1|C2|C3, "mem_load_retired.other_core_l2_hit_hitm"    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCB, 0x10, C0|C1|C2|C3, "mem_load_retired.llc_miss"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCB, 0x40, C0|C1|C2|C3, "mem_load_retired.hit_lfb"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCB, 0x80, C0|C1|C2|C3, "mem_load_retired.dtlb_miss"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCC, 0x01, C0|C1|C2|C3, "fp_mmx_trans.to_fp"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCC, 0x02, C0|C1|C2|C3, "fp_mmx_trans.to_mmx"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCC, 0x03, C0|C1|C2|C3, "fp_mmx_trans.any"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD0, 0x01, C0|C1|C2|C3, "macro_insts.decoded"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD1, 0x02, C0|C1|C2|C3, "uops_decoded.ms"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD1, 0x04, C0|C1|C2|C3, "uops_decoded.esp_folding"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD1, 0x08, C0|C1|C2|C3, "uops_decoded.esp_sync"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD2, 0x01, C0|C1|C2|C3, "rat_stalls.flags"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD2, 0x02, C0|C1|C2|C3, "rat_stalls.registers"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD2, 0x04, C0|C1|C2|C3, "rat_stalls.rob_read_port"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD2, 0x08, C0|C1|C2|C3, "rat_stalls.scoreboard"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD2, 0x0F, C0|C1|C2|C3, "rat_stalls.any"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD4, 0x01, C0|C1|C2|C3, "seg_rename_stalls"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD5, 0x01, C0|C1|C2|C3, "es_reg_renames"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xDB, 0x01, C0|C1|C2|C3, "uop_unfusion"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xE0, 0x01, C0|C1|C2|C3, "br_inst_decoded"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xE5, 0x01, C0|C1|C2|C3, "bpu_missed_call_ret"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xE6, 0x01, C0|C1|C2|C3, "baclear.clear"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xE6, 0x02, C0|C1|C2|C3, "baclear.bad_target"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xE8, 0x01, C0|C1|C2|C3, "bpu_clears.early"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xE8, 0x02, C0|C1|C2|C3, "bpu_clears.late"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x01, C0|C1|C2|C3, "l2_transactions.load"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x02, C0|C1|C2|C3, "l2_transactions.rfo"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x04, C0|C1|C2|C3, "l2_transactions.ifetch"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x08, C0|C1|C2|C3, "l2_transactions.prefetch"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x10, C0|C1|C2|C3, "l2_transactions.l1d_wb"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x20, C0|C1|C2|C3, "l2_transactions.fill"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x40, C0|C1|C2|C3, "l2_transactions.wb"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x80, C0|C1|C2|C3, "l2_transactions.any"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF1, 0x02, C0|C1|C2|C3, "l2_lines_in.s_state"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF1, 0x04, C0|C1|C2|C3, "l2_lines_in.e_state"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF1, 0x07, C0|C1|C2|C3, "l2_lines_in.any"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x01, C0|C1|C2|C3, "l2_lines_out.demand_clean"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x02, C0|C1|C2|C3, "l2_lines_out.demand_dirty"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x04, C0|C1|C2|C3, "l2_lines_out.prefetch_clean"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x08, C0|C1|C2|C3, "l2_lines_out.prefetch_dirty"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x0F, C0|C1|C2|C3, "l2_lines_out.any"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF4, 0x10, C0|C1|C2|C3, "sq_misc.split_lock"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF6, 0x01, C0|C1|C2|C3, "sq_full_stall_cycles"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF7, 0x01, C0|C1|C2|C3, "fp_assist.all"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF7, 0x02, C0|C1|C2|C3, "fp_assist.output"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF7, 0x04, C0|C1|C2|C3, "fp_assist.input"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xFD, 0x01, C0|C1|C2|C3, "simd_int_64.packed_mpy"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xFD, 0x02, C0|C1|C2|C3, "simd_int_64.packed_shift"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xFD, 0x04, C0|C1|C2|C3, "simd_int_64.pack"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xFD, 0x08, C0|C1|C2|C3, "simd_int_64.unpack"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xFD, 0x10, C0|C1|C2|C3, "simd_int_64.packed_logical"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xFD, 0x20, C0|C1|C2|C3, "simd_int_64.packed_arith"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xFD, 0x40, C0|C1|C2|C3, "simd_int_64.shuffle_move"             , 0x0, ATTR_NONE, 0x0 }, \
/* end of #define */

#define   EVENTS_FAM6_MOD46_ONLY                  \
{ 0x0F, 0x01, C0|C1|C2|C3, "mem_uncore_retired.l3_data_miss_unknown"    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0F, 0x80, C0|C1|C2|C3, "mem_uncore_retired.uncacheable"       , 0x0, ATTR_NONE, 0x0 }, \
/* end of #define */

/* Intel Westmere Processor */
/*
 * The Westmere tables are basically from Bug 16173963
 *   libcpc counter names should be based on public Intel documentation -- Westmere
 * and those tables are basically from the
 * Intel SDM, January 2013, Section 19.6, Table 19-13.
 * We omit the Table 19-14 uncore events.
 *
 * Note that the table below includes some events from
 * the Intel SDM that require cmask or attr settings.
 * These events are not in libcpc, which did not include
 * events requiring cmask or attr until Sandy Bridge.
 */

#define EVENTS_FAM6_MOD37                                               \
{ 0x03, 0x02, C0|C1|C2|C3, "load_block.overlap_store"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x04, 0x07, C0|C1|C2|C3, "sb_drain.any"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x05, 0x02, C0|C1|C2|C3, "misalign_mem_ref.store"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x06, 0x04, C0|C1|C2|C3, "store_blocks.at_ret"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x06, 0x08, C0|C1|C2|C3, "store_blocks.l1d_block"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x07, 0x01, C0|C1|C2|C3, "partial_address_alias"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x01, C0|C1|C2|C3, "dtlb_load_misses.any"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x02, C0|C1|C2|C3, "dtlb_load_misses.walk_completed"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x04, C0|C1|C2|C3, "dtlb_load_misses.walk_cycles"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x10, C0|C1|C2|C3, "dtlb_load_misses.stlb_hit"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x20, C0|C1|C2|C3, "dtlb_load_misses.pde_miss"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0B, 0x01, C0|C1|C2|C3, "mem_inst_retired.loads"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0B, 0x02, C0|C1|C2|C3, "mem_inst_retired.stores"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0B, 0x10, C0|C1|C2|C3, "mem_inst_retired.latency_above_threshold"   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0C, 0x01, C0|C1|C2|C3, "mem_store_retired.dtlb_miss"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0E, 0x01, C0|C1|C2|C3, "uops_issued.any"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0E, 0x02, C0|C1|C2|C3, "uops_issued.fused"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0F, 0x01, C0|C1|C2|C3, "mem_uncore_retired.unknown_source"    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0F, 0x80, C0|C1|C2|C3, "mem_uncore_retired.uncacheable"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x01, C0|C1|C2|C3, "fp_comp_ops_exe.x87"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x02, C0|C1|C2|C3, "fp_comp_ops_exe.mmx"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x04, C0|C1|C2|C3, "fp_comp_ops_exe.sse_fp"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x08, C0|C1|C2|C3, "fp_comp_ops_exe.sse2_integer"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x10, C0|C1|C2|C3, "fp_comp_ops_exe.sse_fp_packed"        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x20, C0|C1|C2|C3, "fp_comp_ops_exe.sse_fp_scalar"        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x40, C0|C1|C2|C3, "fp_comp_ops_exe.sse_single_precision" , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x80, C0|C1|C2|C3, "fp_comp_ops_exe.sse_double_precision" , 0x0, ATTR_NONE, 0x0 }, \
{ 0x12, 0x01, C0|C1|C2|C3, "simd_int_128.packed_mpy"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x12, 0x02, C0|C1|C2|C3, "simd_int_128.packed_shift"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x12, 0x04, C0|C1|C2|C3, "simd_int_128.pack"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x12, 0x08, C0|C1|C2|C3, "simd_int_128.unpack"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x12, 0x10, C0|C1|C2|C3, "simd_int_128.packed_logical"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x12, 0x20, C0|C1|C2|C3, "simd_int_128.packed_arith"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x12, 0x40, C0|C1|C2|C3, "simd_int_128.shuffle_move"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x13, 0x01, C0|C1|C2|C3, "load_dispatch.rs"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x13, 0x02, C0|C1|C2|C3, "load_dispatch.rs_delayed"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x13, 0x04, C0|C1|C2|C3, "load_dispatch.mob"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x13, 0x07, C0|C1|C2|C3, "load_dispatch.any"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x14, 0x01, C0|C1|C2|C3, "arith.cycles_div_busy"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x14, 0x02, C0|C1|C2|C3, "arith.mul"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x17, 0x01, C0|C1|C2|C3, "inst_queue_writes"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x18, 0x01, C0|C1|C2|C3, "inst_decoded.dec0"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x19, 0x01, C0|C1|C2|C3, "two_uop_insts_decoded"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x1E, 0x01, C0|C1|C2|C3, "inst_queue_write_cycles"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x20, 0x01, C0|C1|C2|C3, "lsd_overflow"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x01, C0|C1|C2|C3, "l2_rqsts.ld_hit"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x02, C0|C1|C2|C3, "l2_rqsts.ld_miss"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x03, C0|C1|C2|C3, "l2_rqsts.loads"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x04, C0|C1|C2|C3, "l2_rqsts.rfo_hit"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x08, C0|C1|C2|C3, "l2_rqsts.rfo_miss"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x0C, C0|C1|C2|C3, "l2_rqsts.rfos"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x10, C0|C1|C2|C3, "l2_rqsts.ifetch_hit"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x20, C0|C1|C2|C3, "l2_rqsts.ifetch_miss"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x30, C0|C1|C2|C3, "l2_rqsts.ifetches"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x40, C0|C1|C2|C3, "l2_rqsts.prefetch_hit"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x80, C0|C1|C2|C3, "l2_rqsts.prefetch_miss"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xAA, C0|C1|C2|C3, "l2_rqsts.miss"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xC0, C0|C1|C2|C3, "l2_rqsts.prefetches"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xFF, C0|C1|C2|C3, "l2_rqsts.references"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0x01, C0|C1|C2|C3, "l2_data_rqsts.demand.i_state"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0x02, C0|C1|C2|C3, "l2_data_rqsts.demand.s_state"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0x04, C0|C1|C2|C3, "l2_data_rqsts.demand.e_state"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0x08, C0|C1|C2|C3, "l2_data_rqsts.demand.m_state"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0x0F, C0|C1|C2|C3, "l2_data_rqsts.demand.mesi"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0x10, C0|C1|C2|C3, "l2_data_rqsts.prefetch.i_state"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0x20, C0|C1|C2|C3, "l2_data_rqsts.prefetch.s_state"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0x40, C0|C1|C2|C3, "l2_data_rqsts.prefetch.e_state"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0x80, C0|C1|C2|C3, "l2_data_rqsts.prefetch.m_state"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0xF0, C0|C1|C2|C3, "l2_data_rqsts.prefetch.mesi"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x26, 0xFF, C0|C1|C2|C3, "l2_data_rqsts.any"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x01, C0|C1|C2|C3, "l2_write.rfo.i_state"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x02, C0|C1|C2|C3, "l2_write.rfo.s_state"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x08, C0|C1|C2|C3, "l2_write.rfo.m_state"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x0E, C0|C1|C2|C3, "l2_write.rfo.hit"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x0F, C0|C1|C2|C3, "l2_write.rfo.mesi"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x10, C0|C1|C2|C3, "l2_write.lock.i_state"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x20, C0|C1|C2|C3, "l2_write.lock.s_state"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x40, C0|C1|C2|C3, "l2_write.lock.e_state"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x80, C0|C1|C2|C3, "l2_write.lock.m_state"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0xE0, C0|C1|C2|C3, "l2_write.lock.hit"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0xF0, C0|C1|C2|C3, "l2_write.lock.mesi"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x28, 0x01, C0|C1|C2|C3, "l1d_wb_l2.i_state"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x28, 0x02, C0|C1|C2|C3, "l1d_wb_l2.s_state"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x28, 0x04, C0|C1|C2|C3, "l1d_wb_l2.e_state"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x28, 0x08, C0|C1|C2|C3, "l1d_wb_l2.m_state"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x28, 0x0F, C0|C1|C2|C3, "l1d_wb_l2.mesi"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2E, 0x41, C0|C1|C2|C3, "l3_lat_cache.miss"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2E, 0x4F, C0|C1|C2|C3, "l3_lat_cache.reference"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3C, 0x00, C0|C1|C2|C3, "cpu_clk_unhalted.thread_p"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3C, 0x01, C0|C1|C2|C3, "cpu_clk_unhalted.ref_p"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x01, C0|C1|C2|C3, "dtlb_misses.any"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x02, C0|C1|C2|C3, "dtlb_misses.walk_completed"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x04, C0|C1|C2|C3, "dtlb_misses.walk_cycles"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x10, C0|C1|C2|C3, "dtlb_misses.stlb_hit"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x20, C0|C1|C2|C3, "dtlb_misses.pde_miss"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x80, C0|C1|C2|C3, "dtlb_misses.large_walk_completed"     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4C, 0x01, C0|C1      , "load_hit_pre"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4E, 0x01, C0|C1      , "l1d_prefetch.requests"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4E, 0x02, C0|C1      , "l1d_prefetch.miss"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4E, 0x04, C0|C1      , "l1d_prefetch.triggers"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4F, 0x10, C0|C1|C2|C3, "ept.walk_cycles"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x51, 0x01, C0|C1      , "l1d.repl"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x51, 0x02, C0|C1      , "l1d.m_repl"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x51, 0x04, C0|C1      , "l1d.m_evict"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x51, 0x08, C0|C1      , "l1d.m_snoop_evict"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x52, 0x01, C0|C1|C2|C3, "l1d_cache_prefetch_lock_fb_hit"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x01, C0         , "offcore_requests_outstanding.demand.read_data", 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x02, C0         , "offcore_requests_outstanding.demand.read_code", 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x04, C0         , "offcore_requests_outstanding.demand.rfo"    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x08, C0         , "offcore_requests_outstanding.any_read", 0x0, ATTR_NONE, 0x0 }, \
{ 0x63, 0x01, C0|C1      , "cache_lock_cycles.l1d_l2"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x63, 0x02, C0|C1      , "cache_lock_cycles.l1d"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x6C, 0x01, C0|C1|C2|C3, "io_transactions"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x80, 0x01, C0|C1|C2|C3, "l1i.hits"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x80, 0x02, C0|C1|C2|C3, "l1i.misses"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x80, 0x03, C0|C1|C2|C3, "l1i.reads"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x80, 0x04, C0|C1|C2|C3, "l1i.cycles_stalled"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x82, 0x01, C0|C1|C2|C3, "large_itlb.hit"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x01, C0|C1|C2|C3, "itlb_misses.any"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x02, C0|C1|C2|C3, "itlb_misses.walk_completed"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x04, C0|C1|C2|C3, "itlb_misses.walk_cycles"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x10, C0|C1|C2|C3, "itlb_misses.stlb_hit"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x80, C0|C1|C2|C3, "itlb_misses.large_walk_completed"     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x87, 0x01, C0|C1|C2|C3, "ild_stall.lcp"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x87, 0x02, C0|C1|C2|C3, "ild_stall.mru"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x87, 0x04, C0|C1|C2|C3, "ild_stall.iq_full"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x87, 0x08, C0|C1|C2|C3, "ild_stall.regen"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x87, 0x0F, C0|C1|C2|C3, "ild_stall.any"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x01, C0|C1|C2|C3, "br_inst_exec.cond"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x02, C0|C1|C2|C3, "br_inst_exec.direct"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x04, C0|C1|C2|C3, "br_inst_exec.indirect_non_call"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x07, C0|C1|C2|C3, "br_inst_exec.non_calls"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x08, C0|C1|C2|C3, "br_inst_exec.return_near"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x10, C0|C1|C2|C3, "br_inst_exec.direct_near_call"        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x20, C0|C1|C2|C3, "br_inst_exec.indirect_near_call"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x30, C0|C1|C2|C3, "br_inst_exec.near_calls"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x40, C0|C1|C2|C3, "br_inst_exec.taken"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x7F, C0|C1|C2|C3, "br_inst_exec.any"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x01, C0|C1|C2|C3, "br_misp_exec.cond"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x02, C0|C1|C2|C3, "br_misp_exec.direct"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x04, C0|C1|C2|C3, "br_misp_exec.indirect_non_call"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x07, C0|C1|C2|C3, "br_misp_exec.non_calls"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x08, C0|C1|C2|C3, "br_misp_exec.return_near"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x10, C0|C1|C2|C3, "br_misp_exec.direct_near_call"        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x20, C0|C1|C2|C3, "br_misp_exec.indirect_near_call"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x30, C0|C1|C2|C3, "br_misp_exec.near_calls"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x40, C0|C1|C2|C3, "br_misp_exec.taken"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x7F, C0|C1|C2|C3, "br_misp_exec.any"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x01, C0|C1|C2|C3, "resource_stalls.any"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x02, C0|C1|C2|C3, "resource_stalls.load"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x04, C0|C1|C2|C3, "resource_stalls.rs_full"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x08, C0|C1|C2|C3, "resource_stalls.store"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x10, C0|C1|C2|C3, "resource_stalls.rob_full"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x20, C0|C1|C2|C3, "resource_stalls.fpcw"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x40, C0|C1|C2|C3, "resource_stalls.mxcsr"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x80, C0|C1|C2|C3, "resource_stalls.other"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA6, 0x01, C0|C1|C2|C3, "macro_insts.fusions_decoded"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA7, 0x01, C0|C1|C2|C3, "baclear_force_iq"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA8, 0x01, C0|C1|C2|C3, "lsd.uops"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xAE, 0x01, C0|C1|C2|C3, "itlb_flush"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x01, C0|C1|C2|C3, "offcore_requests.demand.read_data"    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x02, C0|C1|C2|C3, "offcore_requests.demand.read_code"    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x04, C0|C1|C2|C3, "offcore_requests.demand.rfo"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x08, C0|C1|C2|C3, "offcore_requests.any.read"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x10, C0|C1|C2|C3, "offcore_requests.any.rfo"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x40, C0|C1|C2|C3, "offcore_requests.l1d_writeback"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x80, C0|C1|C2|C3, "offcore_requests.any"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x01, C0|C1|C2|C3, "uops_executed.port0"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x02, C0|C1|C2|C3, "uops_executed.port1"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x04, C0|C1|C2|C3, "uops_executed.port2_core"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x08, C0|C1|C2|C3, "uops_executed.port3_core"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x10, C0|C1|C2|C3, "uops_executed.port4_core"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x1F, C0|C1|C2|C3, "uops_executed.core_active_cycles_no_port5"  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x20, C0|C1|C2|C3, "uops_executed.port5"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x3F, C0|C1|C2|C3, "uops_executed.core_active_cycles"     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x40, C0|C1|C2|C3, "uops_executed.port015"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x80, C0|C1|C2|C3, "uops_executed.port234"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB2, 0x01, C0|C1|C2|C3, "offcore_requests_sq_full"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB3, 0x01, C0,          "snoopq_requests_outstanding.data"     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB3, 0x02, C0,          "snoopq_requests_outstanding.invalidate"     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB3, 0x04, C0,          "snoopq_requests_outstanding.code"     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB4, 0x01, C0|C1|C2|C3, "snoopq_requests.code"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB4, 0x02, C0|C1|C2|C3, "snoopq_requests.data"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB4, 0x04, C0|C1|C2|C3, "snoopq_requests.invalidate"           , 0x0, ATTR_NONE, 0x0 }, \
/* { 0xB7, 0x01, C0|C1|C2|C3, "off_core_response_0"                  , 0x0, ATTR_NONE, 0x1A6 }, ignore events that require msr_offset */ \
{ 0xB8, 0x01, C0|C1|C2|C3, "snoop_response.hit"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB8, 0x02, C0|C1|C2|C3, "snoop_response.hite"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB8, 0x04, C0|C1|C2|C3, "snoop_response.hitm"                  , 0x0, ATTR_NONE, 0x0 }, \
/* { 0xBB, 0x01, C0|C1|C2|C3, "off_core_response_1"                  , 0x0, ATTR_NONE, 0x1A7 }, ignore events that require msr_offset */ \
{ 0xC0, 0x00, C0|C1|C2|C3, "inst_retired.any_p"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC0, 0x02, C0|C1|C2|C3, "inst_retired.x87"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC0, 0x04, C0|C1|C2|C3, "inst_retired.mmx"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC2, 0x01, C0|C1|C2|C3, "uops_retired.any"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC2, 0x02, C0|C1|C2|C3, "uops_retired.retire_slots"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC2, 0x04, C0|C1|C2|C3, "uops_retired.macro_fused"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC3, 0x01, C0|C1|C2|C3, "machine_clears.cycles"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC3, 0x02, C0|C1|C2|C3, "machine_clears.mem_order"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC3, 0x04, C0|C1|C2|C3, "machine_clears.smc"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x00, C0|C1|C2|C3, "br_inst_retired.all_branches"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x01, C0|C1|C2|C3, "br_inst_retired.conditional"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x02, C0|C1|C2|C3, "br_inst_retired.near_call"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC5, 0x00, C0|C1|C2|C3, "br_misp_retired.all_branches"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC5, 0x01, C0|C1|C2|C3, "br_misp_retired.conditional"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC5, 0x02, C0|C1|C2|C3, "br_misp_retired.near_call"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC5, 0x04, C0|C1|C2|C3, "br_misp_retired.all_branches"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC7, 0x01, C0|C1|C2|C3, "ssex_uops_retired.packed_single"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC7, 0x02, C0|C1|C2|C3, "ssex_uops_retired.scalar_single"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC7, 0x04, C0|C1|C2|C3, "ssex_uops_retired.packed_double"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC7, 0x08, C0|C1|C2|C3, "ssex_uops_retired.scalar_double"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC7, 0x10, C0|C1|C2|C3, "ssex_uops_retired.vector_integer"     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC8, 0x20, C0|C1|C2|C3, "itlb_miss_retired"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCB, 0x01, C0|C1|C2|C3, "mem_load_retired.l1d_hit"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCB, 0x02, C0|C1|C2|C3, "mem_load_retired.l2_hit"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCB, 0x04, C0|C1|C2|C3, "mem_load_retired.llc_unshared_hit"    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCB, 0x08, C0|C1|C2|C3, "mem_load_retired.other_core_l2_hit_hitm"    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCB, 0x10, C0|C1|C2|C3, "mem_load_retired.llc_miss"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCB, 0x40, C0|C1|C2|C3, "mem_load_retired.hit_lfb"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCB, 0x80, C0|C1|C2|C3, "mem_load_retired.dtlb_miss"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCC, 0x01, C0|C1|C2|C3, "fp_mmx_trans.to_fp"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCC, 0x02, C0|C1|C2|C3, "fp_mmx_trans.to_mmx"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCC, 0x03, C0|C1|C2|C3, "fp_mmx_trans.any"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD0, 0x01, C0|C1|C2|C3, "macro_insts.decoded"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD1, 0x01, C0|C1|C2|C3, "uops_decoded.stall_cycles"            , 0x1, ATTR_INV , 0x0 }, \
{ 0xD1, 0x02, C0|C1|C2|C3, "uops_decoded.ms"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD1, 0x04, C0|C1|C2|C3, "uops_decoded.esp_folding"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD1, 0x08, C0|C1|C2|C3, "uops_decoded.esp_sync"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD2, 0x01, C0|C1|C2|C3, "rat_stalls.flags"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD2, 0x02, C0|C1|C2|C3, "rat_stalls.registers"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD2, 0x04, C0|C1|C2|C3, "rat_stalls.rob_read_port"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD2, 0x08, C0|C1|C2|C3, "rat_stalls.scoreboard"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD2, 0x0F, C0|C1|C2|C3, "rat_stalls.any"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD4, 0x01, C0|C1|C2|C3, "seg_rename_stalls"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD5, 0x01, C0|C1|C2|C3, "es_reg_renames"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xDB, 0x01, C0|C1|C2|C3, "uop_unfusion"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xE0, 0x01, C0|C1|C2|C3, "br_inst_decoded"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xE5, 0x01, C0|C1|C2|C3, "bpu_missed_call_ret"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xE6, 0x01, C0|C1|C2|C3, "baclear.clear"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xE6, 0x02, C0|C1|C2|C3, "baclear.bad_target"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xE8, 0x02, C0|C1|C2|C3, "bpu_clears.late"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xEC, 0x01, C0|C1|C2|C3, "thread_active"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x01, C0|C1|C2|C3, "l2_transactions.load"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x02, C0|C1|C2|C3, "l2_transactions.rfo"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x04, C0|C1|C2|C3, "l2_transactions.ifetch"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x08, C0|C1|C2|C3, "l2_transactions.prefetch"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x10, C0|C1|C2|C3, "l2_transactions.l1d_wb"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x20, C0|C1|C2|C3, "l2_transactions.fill"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x40, C0|C1|C2|C3, "l2_transactions.wb"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x80, C0|C1|C2|C3, "l2_transactions.any"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF1, 0x02, C0|C1|C2|C3, "l2_lines_in.s_state"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF1, 0x04, C0|C1|C2|C3, "l2_lines_in.e_state"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF1, 0x07, C0|C1|C2|C3, "l2_lines_in.any"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x01, C0|C1|C2|C3, "l2_lines_out.demand_clean"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x02, C0|C1|C2|C3, "l2_lines_out.demand_dirty"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x04, C0|C1|C2|C3, "l2_lines_out.prefetch_clean"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x08, C0|C1|C2|C3, "l2_lines_out.prefetch_dirty"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x0F, C0|C1|C2|C3, "l2_lines_out.any"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF4, 0x04, C0|C1|C2|C3, "sq_misc.lru_hints"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF4, 0x10, C0|C1|C2|C3, "sq_misc.split_lock"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF6, 0x01, C0|C1|C2|C3, "sq_full_stall_cycles"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF7, 0x01, C0|C1|C2|C3, "fp_assist.all"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF7, 0x02, C0|C1|C2|C3, "fp_assist.output"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF7, 0x04, C0|C1|C2|C3, "fp_assist.input"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xFD, 0x01, C0|C1|C2|C3, "simd_int_64.packed_mpy"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xFD, 0x02, C0|C1|C2|C3, "simd_int_64.packed_shift"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xFD, 0x04, C0|C1|C2|C3, "simd_int_64.pack"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xFD, 0x08, C0|C1|C2|C3, "simd_int_64.unpack"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xFD, 0x10, C0|C1|C2|C3, "simd_int_64.packed_logical"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xFD, 0x20, C0|C1|C2|C3, "simd_int_64.packed_arith"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xFD, 0x40, C0|C1|C2|C3, "simd_int_64.shuffle_move"             , 0x0, ATTR_NONE, 0x0 }, \
/* end of #define */

/*
 * This special omission of the following events from Model 47
 * is due to usr/src/uts/intel/pcbe/wm_pcbe.h .  There seems
 * to be no substantiation for this treatment in the Intel SDM.
 */
#define       EVENTS_FAM6_MOD37_ALSO                                    \
{ 0x0F, 0x02, C0|C1|C2|C3, "mem_uncore_retired.other_core_l2_hit" , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0F, 0x04, C0|C1|C2|C3, "mem_uncore_retired.remote_hitm"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0F, 0x08, C0|C1|C2|C3, "mem_uncore_retired.local_dram_remote_cache_hit", 0x0, ATTR_NONE, 0x0 },\
{ 0x0F, 0x10, C0|C1|C2|C3, "mem_uncore_retired.remote_dram"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0F, 0x20, C0|C1|C2|C3, "mem_uncore_retired.other_llc_miss"    , 0x0, ATTR_NONE, 0x0 }, \
/* end of #define */

/* Intel Sandy Bridge Processor */
/*
 * The Sandy Bridge tables are basically from Bug 16457080
 *   libcpc counter names should be based on public Intel documentation -- Sandy Bridge
 * and those tables are basically from the
 * Intel SDM, January 2013, Section 19.4, Table 19-7.
 * Additionally, there are
 *   Table 19-8.  Model 42 only.
 *   Table 19-9.  Model 45 only.
 * We omit the Table 19-10 uncore events.
 */

#define EVENTS_FAM6_MOD42                                               \
{ 0x03, 0x01, C_ALL, "ld_blocks.data_unknown"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x03, 0x02, C_ALL, "ld_blocks.store_forward"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x03, 0x08, C_ALL, "ld_blocks.no_sr"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x03, 0x10, C_ALL, "ld_blocks.all_block"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x05, 0x01, C_ALL, "misalign_mem_ref.loads"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x05, 0x02, C_ALL, "misalign_mem_ref.stores"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x07, 0x01, C_ALL, "ld_blocks_partial.address_alias"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x07, 0x08, C_ALL, "ld_blocks_partial.all_sta_block"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x01, C_ALL, "dtlb_load_misses.miss_causes_a_walk"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x02, C_ALL, "dtlb_load_misses.walk_completed"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x04, C_ALL, "dtlb_load_misses.walk_duration"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x10, C_ALL, "dtlb_load_misses.stlb_hit"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0D, 0x03, C_ALL, "int_misc.recovery_cycles"                          , 0x1, ATTR_NONE, 0x0 }, \
{ 0x0D, 0x03, C_ALL, "int_misc.recovery_stalls_count"                    , 0x1, ATTR_EDGE, 0x0 }, \
{ 0x0D, 0x40, C_ALL, "int_misc.rat_stall_cycles"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0E, 0x01, C_ALL, "uops_issued.any"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0E, 0x01, C_ALL, "uops_issued.stall_cycles"                          , 0x1, ATTR_INV , 0x0 }, \
{ 0x0E, 0x01, C_ALL, "uops_issued.core_stall_cycles"                     , 0x1, ATTR_INV | ATTR_ANY, 0x0 }, \
{ 0x10, 0x01, C_ALL, "fp_comp_ops_exe.x87"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x10, C_ALL, "fp_comp_ops_exe.sse_fp_packed_double"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x20, C_ALL, "fp_comp_ops_exe.sse_fp_scalar_single"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x40, C_ALL, "fp_comp_ops_exe.sse_packed_single"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x80, C_ALL, "fp_comp_ops_exe.sse_scalar_double"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x11, 0x01, C_ALL, "simd_fp_256.packed_single"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x11, 0x02, C_ALL, "simd_fp_256.packed_double"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x14, 0x01, C_ALL, "arith.fpu_div_active"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x14, 0x01, C_ALL, "arith.fpu_div"                                     , 0x1, ATTR_EDGE, 0x0 }, \
{ 0x17, 0x01, C_ALL, "insts_written_to_iq.insts"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x01, C_ALL, "l2_rqsts.demand_data_rd_hit"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x03, C_ALL, "l2_rqsts.all_demand_data_rd"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x04, C_ALL, "l2_rqsts.rfo_hits"                                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x08, C_ALL, "l2_rqsts.rfo_miss"                                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x0C, C_ALL, "l2_rqsts.all_rfo"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x10, C_ALL, "l2_rqsts.code_rd_hit"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x20, C_ALL, "l2_rqsts.code_rd_miss"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x30, C_ALL, "l2_rqsts.all_code_rd"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x40, C_ALL, "l2_rqsts.pf_hit"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x80, C_ALL, "l2_rqsts.pf_miss"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xC0, C_ALL, "l2_rqsts.all_pf"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x01, C_ALL, "l2_store_lock_rqsts.miss"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x04, C_ALL, "l2_store_lock_rqsts.hit_e"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x08, C_ALL, "l2_store_lock_rqsts.hit_m"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x0F, C_ALL, "l2_store_lock_rqsts.all"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x28, 0x01, C_ALL, "l2_l1d_wb_rqsts.miss"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x28, 0x02, C_ALL, "l2_l1d_wb_rqsts.hit_s"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x28, 0x04, C_ALL, "l2_l1d_wb_rqsts.hit_e"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x28, 0x08, C_ALL, "l2_l1d_wb_rqsts.hit_m"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x28, 0x0F, C_ALL, "l2_l1d_wb_rqsts.all"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2E, 0x41, C_ALL, "longest_lat_cache.miss"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2E, 0x4F, C_ALL, "longest_lat_cache.reference"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3C, 0x00, C_ALL, "cpu_clk_unhalted.thread_p"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3C, 0x01, C_ALL, "cpu_clk_thread_unhalted.ref_xclk"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x48, 0x01, C2   , "l1d_pend_miss.pending"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x48, 0x01, C2   , "l1d_pend_miss.pending_cycles"                      , 0x1, ATTR_NONE, 0x0 }, \
{ 0x48, 0x01, C2   , "l1d_pend_miss.occurrences"                         , 0x1, ATTR_EDGE, 0x0 }, \
{ 0x49, 0x01, C_ALL, "dtlb_store_misses.miss_causes_a_walk"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x02, C_ALL, "dtlb_store_misses.walk_completed"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x04, C_ALL, "dtlb_store_misses.walk_duration"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x10, C_ALL, "dtlb_store_misses.stlb_hit"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4C, 0x01, C_ALL, "load_hit_pre.sw_pf"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4C, 0x02, C_ALL, "load_hit_pre.hw_pf"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4E, 0x02, C_ALL, "hw_pre_req.dl1_miss"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x51, 0x01, C_ALL, "l1d.replacement"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x51, 0x02, C_ALL, "l1d.allocated_in_m"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x51, 0x04, C_ALL, "l1d.eviction"                                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x51, 0x08, C_ALL, "l1d.all_m_replacement"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x59, 0x20, C_ALL, "partial_rat_stalls.flags_merge_uop"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x59, 0x20, C_ALL, "partial_rat_stalls.flags_merge_uop_cycles"         , 0x1, ATTR_NONE, 0x0 }, \
{ 0x59, 0x40, C_ALL, "partial_rat_stalls.slow_lea_window"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x59, 0x80, C_ALL, "partial_rat_stalls.mul_single_uop"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x5B, 0x0C, C0|C1|C2|C3, "resource_stalls2.all_fl_empty"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x5B, 0x0F, C_ALL, "resource_stalls2.all_prf_control"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x5B, 0x40, C_ALL, "resource_stalls2.bob_full"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x5B, 0x4F, C_ALL, "resource_stalls2.ooo_rsrc"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x5C, 0x01, C_ALL, "cpl_cycles.ring0"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x5C, 0x01, C_ALL, "cpl_cycles.ring0_transition"                       , 0x0, ATTR_EDGE, 0x0 }, \
{ 0x5C, 0x02, C_ALL, "cpl_cycles.ring123"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x5E, 0x01, C_ALL, "rs_events.empty_cycles"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x01, C_ALL, "offcore_requests_outstanding.demand_data_rd"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x01, C_ALL, "offcore_requests_outstanding.demand_data_rd_cycles", 0x1, ATTR_NONE, 0x0 }, \
{ 0x60, 0x04, C_ALL, "offcore_requests_outstanding.demand_rfo"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x04, C_ALL, "offcore_requests_outstanding.demand_rfo_cycles"    , 0x1, ATTR_NONE, 0x0 }, \
{ 0x60, 0x08, C_ALL, "offcore_requests_outstanding.all_data_rd"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x08, C_ALL, "offcore_requests_outstanding.all_data_rd_cycles"   , 0x1, ATTR_NONE, 0x0 }, \
{ 0x63, 0x01, C_ALL, "lock_cycles.split_lock_uc_lock_duration"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x63, 0x02, C_ALL, "lock_cycles.cache_lock_duration"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x02, C_ALL, "idq.empty"                                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x04, C_ALL, "idq.mite_uops"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x04, C_ALL, "idq.mite_cycles"                                   , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x08, C_ALL, "idq.dsb_uops"                                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x08, C_ALL, "idq.dsb_cycles"                                    , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x10, C_ALL, "idq.ms_dsb_uops"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x10, C_ALL, "idq.ms_dsb_cycles"                                 , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x10, C_ALL, "idq.ms_dsb_activations"                            , 0x1, ATTR_EDGE, 0x0 }, \
{ 0x79, 0x20, C_ALL, "idq.ms_mite_uops"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x20, C_ALL, "idq.ms_mite_cycles"                                , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x30, C_ALL, "idq.ms_uops"                                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x30, C_ALL, "idq.ms_cycles"                                     , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x18, C_ALL, "idq.all_dsb_uops"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x18, C_ALL, "idq.all_dsb_cycles"                                , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x24, C_ALL, "idq.all_mite_uops"                                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x24, C_ALL, "idq.all_mite_cycles"                               , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x3C, C_ALL, "idq.mite_all_uops"                                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x3C, C_ALL, "idq.mite_all_cycles"                               , 0x1, ATTR_NONE, 0x0 }, \
{ 0x80, 0x02, C_ALL, "icache.misses"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x01, C_ALL, "itlb_misses.miss_causes_a_walk"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x02, C_ALL, "itlb_misses.walk_completed"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x04, C_ALL, "itlb_misses.walk_duration"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x10, C_ALL, "itlb_misses.stlb_hit"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x87, 0x01, C_ALL, "ild_stall.lcp"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x87, 0x04, C_ALL, "ild_stall.iq_full"                                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x41, C_ALL, "br_inst_exec.nontaken_cond"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x81, C_ALL, "br_inst_exec.taken_cond"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x82, C_ALL, "br_inst_exec.taken_direct_jmp"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x84, C_ALL, "br_inst_exec.taken_indirect_jmp_non_call_ret"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x88, C_ALL, "br_inst_exec.taken_return_near"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x90, C_ALL, "br_inst_exec.taken_direct_near_call"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0xA0, C_ALL, "br_inst_exec.taken_indirect_near_call"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0xC1, C_ALL, "br_inst_exec.all_cond"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0xFF, C_ALL, "br_inst_exec.all_branches"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x41, C_ALL, "br_misp_exec.nontaken_cond"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x81, C_ALL, "br_misp_exec.taken_cond"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x84, C_ALL, "br_misp_exec.taken_indirect_jmp_non_call_ret"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x88, C_ALL, "br_misp_exec.taken_return_near"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x90, C_ALL, "br_misp_exec.taken_direct_near_call"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0xA0, C_ALL, "br_misp_exec.taken_indirect_near_call"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0xC1, C_ALL, "br_misp_exec.all_cond"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0xFF, C_ALL, "br_misp_exec.all_branches"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x9C, 0x01, C_ALL, "idq_uops_not_delivered.core"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x01, C_ALL, "uops_dispatched_port.port_0"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x02, C_ALL, "uops_dispatched_port.port_1"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x04, C_ALL, "uops_dispatched_port.port_2_ld"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x08, C_ALL, "uops_dispatched_port.port_2_sta"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x0C, C_ALL, "uops_dispatched_port.port_2"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x10, C_ALL, "uops_dispatched_port.port_3_ld"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x20, C_ALL, "uops_dispatched_port.port_3_sta"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x30, C_ALL, "uops_dispatched_port.port_3"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x40, C_ALL, "uops_dispatched_port.port_4"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x80, C_ALL, "uops_dispatched_port.port_5"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x01, C_ALL, "resource_stalls.any"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x02, C_ALL, "resource_stalls.lb"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x04, C_ALL, "resource_stalls.rs"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x08, C_ALL, "resource_stalls.sb"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x10, C_ALL, "resource_stalls.rob"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x20, C_ALL, "resource_stalls.fcsw"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x40, C_ALL, "resource_stalls.mxcsr"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x80, C_ALL, "resource_stalls.other"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x02, C2   , "cycle_activity.cycles_l1d_pending"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x01, C_ALL, "cycle_activity.cycles_l2_pending"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x04, C0|C1|C2|C3, "cycle_activity.cycles_no_dispatch"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xAB, 0x01, C_ALL, "dsb2mite_switches.count"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xAB, 0x02, C_ALL, "dsb2mite_switches.penalty_cycles"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xAC, 0x02, C_ALL, "dsb_fill.other_cancel"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xAC, 0x08, C_ALL, "dsb_fill.exceed_dsb_lines"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xAC, 0x0A, C_ALL, "dsb_fill.all_cancel"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xAE, 0x01, C_ALL, "itlb.itlb_flush"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x01, C_ALL, "offcore_requests.demand_data_rd"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x04, C_ALL, "offcore_requests.demand_rfo"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x08, C_ALL, "offcore_requests.all_data_rd"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x01, C0|C1|C2|C3, "uops_dispatched.thread"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x01, C0|C1|C2|C3, "uops_dispatched.stall_cycles"                , 0x1, ATTR_INV , 0x0 }, \
{ 0xB1, 0x02, C_ALL, "uops_dispatched.core"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB2, 0x01, C_ALL, "offcore_requests_buffer.sq_full"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB6, 0x01, C_ALL, "agu_bypass_cancel.count"                           , 0x0, ATTR_NONE, 0x0 }, \
/* { 0xB7, 0x01, C_ALL, "off_core_response_0"                               , 0x0, ATTR_NONE, 0x1A6 }, ignore events that require msr_offset */ \
/* { 0xBB, 0x01, C_ALL, "off_core_response_1"                               , 0x0, ATTR_NONE, 0x1A7 }, ignore events that require msr_offset */ \
{ 0xBD, 0x01, C_ALL, "tlb_flush.dtlb_thread"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xBD, 0x20, C_ALL, "tlb_flush.stlb_any"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xBF, 0x05, C_ALL, "l1d_blocks.bank_conflict_cycles"                   , 0x1, ATTR_NONE, 0x0 }, \
{ 0xC0, 0x00, C_ALL, "inst_retired.any_p"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC0, 0x01,    C1, "inst_retired.prec_dist"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC1, 0x02, C_ALL, "other_assists.itlb_miss_retired"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC1, 0x08, C_ALL, "other_assists.avx_store"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC1, 0x10, C_ALL, "other_assists.avx_to_sse"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC1, 0x20, C_ALL, "other_assists.sse_to_avx"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC2, 0x01, C_ALL, "uops_retired.all"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC2, 0x01, C_ALL, "uops_retired.active_cycles"                        , 0x1, ATTR_NONE, 0x0 }, \
{ 0xC2, 0x01, C_ALL, "uops_retired.stall_cycles"                         , 0x1, ATTR_INV , 0x0 }, \
{ 0xC2, 0x02, C_ALL, "uops_retired.retire_slots"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC3, 0x02, C_ALL, "machine_clears.memory_ordering"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC3, 0x04, C_ALL, "machine_clears.smc"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC3, 0x20, C_ALL, "machine_clears.maskmov"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x00, C_ALL, "br_inst_retired.all_branches"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x01, C_ALL, "br_inst_retired.conditional"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x02, C_ALL, "br_inst_retired.near_call"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x04, C_ALL, "br_inst_retired.all_branches"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x08, C_ALL, "br_inst_retired.near_return"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x10, C_ALL, "br_inst_retired.not_taken"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x20, C_ALL, "br_inst_retired.near_taken"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x40, C_ALL, "br_inst_retired.far_branch"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC5, 0x00, C_ALL, "br_misp_retired.all_branches"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC5, 0x00, C_ALL, "br_misp_retired.all_branches"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC5, 0x01, C_ALL, "br_misp_retired.conditional"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC5, 0x02, C_ALL, "br_misp_retired.near_call"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC5, 0x04, C_ALL, "br_misp_retired.all_branches"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC5, 0x10, C_ALL, "br_misp_retired.not_taken"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC5, 0x20, C_ALL, "br_misp_retired.taken"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCA, 0x02, C_ALL, "fp_assist.x87_output"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCA, 0x04, C_ALL, "fp_assist.x87_input"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCA, 0x08, C_ALL, "fp_assist.simd_output"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCA, 0x10, C_ALL, "fp_assist.simd_input"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCA, 0x1E, C_ALL, "fp_assist.any"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCC, 0x20, C_ALL, "rob_misc_events.lbr_inserts"                       , 0x0, ATTR_NONE, 0x0 }, \
/* { 0xCD, 0x01, C3, "mem_trans_retired.load_latency"                    , 0x0, ATTR_NONE, 0x3F6 }, ignore events that require msr_offset */ /* See Section "MSR_PEBS_LD_LAT_THRESHOLD" */ \
{ 0xCD, 0x02,    C3, "mem_trans_retired.precise_store"                   , 0x0, ATTR_NONE, 0x0 }, /* See Section "Precise Store Facility" */ \
{ 0xD0, 0x11, C_ALL, "mem_uops_retired.stlb_miss_loads"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD0, 0x12, C_ALL, "mem_uops_retired.stlb_miss_stores"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD0, 0x21, C_ALL, "mem_uops_retired.lock_loads"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD0, 0x22, C_ALL, "mem_uops_retired.lock_stores"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD0, 0x41, C_ALL, "mem_uops_retired.split_loads"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD0, 0x42, C_ALL, "mem_uops_retired.split_stores"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD0, 0x81, C_ALL, "mem_uops_retired.all_loads"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD0, 0x82, C_ALL, "mem_uops_retired.all_stores"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD1, 0x01, C0|C1|C2|C3, "mem_load_uops_retired.l1_hit"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD1, 0x02, C_ALL, "mem_load_uops_retired.l2_hit"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD1, 0x04, C_ALL, "mem_load_uops_retired.llc_hit"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD1, 0x20, C_ALL, "mem_load_uops_retired.llc_miss"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD1, 0x40, C_ALL, "mem_load_uops_retired.hit_lfb"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD2, 0x01, C_ALL, "mem_load_uops_llc_hit_retired.xsnp_miss"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD2, 0x02, C_ALL, "mem_load_uops_llc_hit_retired.xsnp_hit"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD2, 0x04, C_ALL, "mem_load_uops_llc_hit_retired.xsnp_hitm"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD2, 0x08, C_ALL, "mem_load_uops_llc_hit_retired.xsnp_none"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xE6, 0x01, C_ALL, "baclears.any"                                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x01, C_ALL, "l2_trans.demand_data_rd"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x02, C_ALL, "l2_trans.rfo"                                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x04, C_ALL, "l2_trans.code_rd"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x08, C_ALL, "l2_trans.all_pf"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x10, C_ALL, "l2_trans.l1d_wb"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x20, C_ALL, "l2_trans.l2_fill"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x40, C_ALL, "l2_trans.l2_wb"                                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x80, C_ALL, "l2_trans.all_requests"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF1, 0x01, C_ALL, "l2_lines_in.i"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF1, 0x02, C_ALL, "l2_lines_in.s"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF1, 0x04, C_ALL, "l2_lines_in.e"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF1, 0x07, C_ALL, "l2_lines_in.all"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x01, C_ALL, "l2_lines_out.demand_clean"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x02, C_ALL, "l2_lines_out.demand_dirty"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x04, C_ALL, "l2_lines_out.pf_clean"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x08, C_ALL, "l2_lines_out.pf_dirty"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x0A, C_ALL, "l2_lines_out.dirty_all"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF4, 0x10, C_ALL, "sq_misc.split_lock"                                , 0x0, ATTR_NONE, 0x0 }, \
/* end of #define */

#define EVENTS_FAM6_MOD42_ONLY \
{ 0xD4, 0x02, C0|C1|C2|C3, "mem_load_uops_misc_retired.llc_miss"         , 0x0, ATTR_NONE, 0x0 }, \
/* end of #define */

#define EVENTS_FAM6_MOD45_ONLY \
/* { 0xD3, 0x01, C_ALL, "mem_load_uops_llc_miss_retired.local_dram"   , 0x0, ATTR_NONE, 0x3C9 }, ignore events that require msr_offset */ \
/* { 0xD3, 0x04, C_ALL, "mem_load_uops_llc_miss_retired.remote_dram"  , 0x0, ATTR_NONE, 0x3C9 }, ignore events that require msr_offset */ \
/* end of #define */

/* Intel Ivy Bridge Processor */
/*
 * The Ivy Bridge tables are basically from Bug 16457100
 *   libcpc counter names should be based on public Intel documentation -- Ivy Bridge
 * and those tables are basically from the
 * Intel SDM, January 2013, Section 19.3, Table 19-5.
 * Additionally, there is
 *   Table 19-6.  Model 62 only.
 */

#define EVENTS_FAM6_MOD58 \
{ 0x03, 0x02, C_ALL, "ld_blocks.store_forward"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x05, 0x01, C_ALL, "misalign_mem_ref.loads"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x05, 0x02, C_ALL, "misalign_mem_ref.stores"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x07, 0x01, C_ALL, "ld_blocks_partial.address_alias"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x81, C_ALL, "dtlb_load_misses.miss_causes_a_walk"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x82, C_ALL, "dtlb_load_misses.walk_completed"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x84, C_ALL, "dtlb_load_misses.walk_duration"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0E, 0x01, C_ALL, "uops_issued.any"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0E, 0x01, C_ALL, "uops_issued.stall_cycles"                          , 0x1, ATTR_INV , 0x0 }, \
{ 0x0E, 0x01, C_ALL, "uops_issued.core_stall_cycles"                     , 0x1, ATTR_INV | ATTR_ANY, 0x0 }, \
{ 0x0E, 0x10, C_ALL, "uops_issued.flags_merge"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0E, 0x20, C_ALL, "uops_issued.slow_lea"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0E, 0x40, C_ALL, "uops_issued.sIngle_mul"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x01, C_ALL, "fp_comp_ops_exe.x87"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x10, C_ALL, "fp_comp_ops_exe.sse_fp_packed_double"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x20, C_ALL, "fp_comp_ops_exe.sse_fp_scalar_single"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x40, C_ALL, "fp_comp_ops_exe.sse_packed_single"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x10, 0x80, C_ALL, "fp_comp_ops_exe.sse_scalar_double"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x11, 0x01, C_ALL, "simd_fp_256.packed_single"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x11, 0x02, C_ALL, "simd_fp_256.packed_double"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x14, 0x01, C_ALL, "arith.fpu_div_active"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x14, 0x01, C_ALL, "arith.fpu_div"                                     , 0x1, ATTR_EDGE, 0x0 }, \
{ 0x24, 0x01, C_ALL, "l2_rqsts.demand_data_rd_hit"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x03, C_ALL, "l2_rqsts.all_demand_data_rd"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x04, C_ALL, "l2_rqsts.rfo_hits"                                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x08, C_ALL, "l2_rqsts.rfo_miss"                                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x0C, C_ALL, "l2_rqsts.all_rfo"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x10, C_ALL, "l2_rqsts.code_rd_hit"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x20, C_ALL, "l2_rqsts.code_rd_miss"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x30, C_ALL, "l2_rqsts.all_code_rd"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x40, C_ALL, "l2_rqsts.pf_hit"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x80, C_ALL, "l2_rqsts.pf_miss"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xC0, C_ALL, "l2_rqsts.all_pf"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x01, C_ALL, "l2_store_lock_rqsts.miss"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x08, C_ALL, "l2_store_lock_rqsts.hit_m"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x0F, C_ALL, "l2_store_lock_rqsts.all"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x28, 0x01, C_ALL, "l2_l1d_wb_rqsts.miss"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x28, 0x04, C_ALL, "l2_l1d_wb_rqsts.hit_e"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x28, 0x08, C_ALL, "l2_l1d_wb_rqsts.hit_m"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x28, 0x0F, C_ALL, "l2_l1d_wb_rqsts.all"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2E, 0x41, C_ALL, "longest_lat_cache.miss"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2E, 0x4F, C_ALL, "longest_lat_cache.reference"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3C, 0x00, C_ALL, "cpu_clk_unhalted.thread_p"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3C, 0x01, C_ALL, "cpu_clk_thread_unhalted.ref_xclk"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x48, 0x01, C(2), "l1d_pend_miss.pending"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x48, 0x01, C(2), "l1d_pend_miss.pending_cycles"                       , 0x1, ATTR_NONE, 0x0 }, \
{ 0x48, 0x01, C(2), "l1d_pend_miss.occurrences"                          , 0x1, ATTR_EDGE, 0x0 }, \
{ 0x49, 0x01, C_ALL, "dtlb_store_misses.miss_causes_a_walk"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x02, C_ALL, "dtlb_store_misses.walk_completed"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x04, C_ALL, "dtlb_store_misses.walk_duration"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x10, C_ALL, "dtlb_store_misses.stlb_hit"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4C, 0x01, C_ALL, "load_hit_pre.sw_pf"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4C, 0x02, C_ALL, "load_hit_pre.hw_pf"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x51, 0x01, C_ALL, "l1d.replacement"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x58, 0x04, C_ALL, "move_elimination.int_not_eliminated"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x58, 0x08, C_ALL, "move_elimination.simd_not_eliminated"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x58, 0x01, C_ALL, "move_elimination.int_eliminated"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x58, 0x02, C_ALL, "move_elimination.simd_eliminated"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x5C, 0x01, C_ALL, "cpl_cycles.ring0"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x5C, 0x01, C_ALL, "cpl_cycles.ring0_trans"                            , 0x0, ATTR_EDGE, 0x0 }, \
{ 0x5C, 0x02, C_ALL, "cpl_cycles.ring123"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x5E, 0x01, C_ALL, "rs_events.empty_cycles"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x5F, 0x04, C_ALL, "dtlb_load_misses.stlb_hit"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x01, C_ALL, "offcore_requests_outstanding.demand_data_rd"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x01, C_ALL, "offcore_requests_outstanding.demand_data_rd_cycles", 0x1, ATTR_NONE, 0x0 }, \
{ 0x60, 0x02, C_ALL, "offcore_requests_outstanding.demand_code_rd"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x02, C_ALL, "offcore_requests_outstanding.demand_code_rd_cycles", 0x1, ATTR_NONE, 0x0 }, \
{ 0x60, 0x04, C_ALL, "offcore_requests_outstanding.demand_rfo"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x04, C_ALL, "offcore_requests_outstanding.demand_rfo_cycles"    , 0x1, ATTR_NONE, 0x0 }, \
{ 0x60, 0x08, C_ALL, "offcore_requests_outstanding.all_data_rd"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x08, C_ALL, "offcore_requests_outstanding.all_data_rd_cycles"   , 0x1, ATTR_NONE, 0x0 }, \
{ 0x63, 0x01, C_ALL, "lock_cycles.split_lock_uc_lock_duration"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x63, 0x02, C_ALL, "lock_cycles.cache_lock_duration"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x02, C_ALL, "idq.empty"                                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x04, C_ALL, "idq.mite_uops"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x04, C_ALL, "idq.mite_cycles"                                   , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x08, C_ALL, "idq.dsb_uops"                                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x08, C_ALL, "idq.dsb_cycles"                                    , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x10, C_ALL, "idq.ms_dsb_uops"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x10, C_ALL, "idq.ms_dsb_cycles"                                 , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x10, C_ALL, "idq.ms_dsb_activations"                            , 0x1, ATTR_EDGE, 0x0 }, \
{ 0x79, 0x18, C_ALL, "idq.all_dsb_uops"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x18, C_ALL, "idq.all_dsb_cycles"                                , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x18, C_ALL, "idq.all_dsb_cycles_any_uops" /* synonym, from Intel SDM */   , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x18, C_ALL, "idq.all_dsb_cycles_4_uops"                         , 0x4, ATTR_NONE, 0x0 }, \
{ 0x79, 0x20, C_ALL, "idq.ms_mite_cycles"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x20, C_ALL, "idq.ms_mite_cycles"                                , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x24, C_ALL, "idq.all_mite_uops"                                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x24, C_ALL, "idq.all_mite_cycles"                               , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x24, C_ALL, "idq.all_mite_cycles_any_uops" /* synonym, from Intel SDM */  , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x24, C_ALL, "idq.all_mite_cycles_4_uops"                        , 0x4, ATTR_NONE, 0x0 }, \
{ 0x79, 0x30, C_ALL, "idq.ms_uops"                                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x30, C_ALL, "idq.ms_cycles"                                     , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x3C, C_ALL, "idq.mite_all_uops"  /* weird name suggested by Intel docs */   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x3C, C_ALL, "idq.mite_all_cycles"                               , 0x1, ATTR_NONE, 0x0 }, \
{ 0x80, 0x02, C_ALL, "icache.misses"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x01, C_ALL, "itlb_misses.miss_causes_a_walk"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x02, C_ALL, "itlb_misses.walk_completed"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x04, C_ALL, "itlb_misses.walk_duration"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x10, C_ALL, "itlb_misses.stlb_hit"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x87, 0x01, C_ALL, "ild_stall.lcp"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x87, 0x04, C_ALL, "ild_stall.iq_full"                                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x41, C_ALL, "br_inst_exec.nontaken_cond"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x81, C_ALL, "br_inst_exec.taken_cond"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x82, C_ALL, "br_inst_exec.taken_direct_jmp"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x84, C_ALL, "br_inst_exec.taken_indirect_jmp_non_call_ret"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x88, C_ALL, "br_inst_exec.taken_return_near"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x90, C_ALL, "br_inst_exec.taken_direct_near_call"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0xA0, C_ALL, "br_inst_exec.taken_indirect_near_call"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0xFF, C_ALL, "br_inst_exec.all_branches"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x41, C_ALL, "br_misp_exec.nontaken_cond"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x81, C_ALL, "br_misp_exec.taken_cond"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x84, C_ALL, "br_misp_exec.taken_indirect_jmp_non_call_ret"      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x88, C_ALL, "br_misp_exec.taken_return_near"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x90, C_ALL, "br_misp_exec.taken_direct_near_call"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0xA0, C_ALL, "br_misp_exec.taken_indirect_near_call"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0xFF, C_ALL, "br_misp_exec.all_branches"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x9C, 0x01, C_ALL, "idq_uops_not_delivered.core"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x01, C_ALL, "uops_dispatched_port.port_0"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x02, C_ALL, "uops_dispatched_port.port_1"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x04, C_ALL, "uops_dispatched_port.port_2_ld"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x08, C_ALL, "uops_dispatched_port.port_2_sta"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x0C, C_ALL, "uops_dispatched_port.port_2"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x10, C_ALL, "uops_dispatched_port.port_3_ld"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x20, C_ALL, "uops_dispatched_port.port_3_sta"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x30, C_ALL, "uops_dispatched_port.port_3"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x40, C_ALL, "uops_dispatched_port.port_4"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x80, C_ALL, "uops_dispatched_port.port_5"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x01, C_ALL, "resource_stalls.any"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x04, C_ALL, "resource_stalls.rs"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x08, C_ALL, "resource_stalls.sb"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x10, C_ALL, "resource_stalls.rob"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x01, C_ALL, "cycle_activity.cycles_l2_pending"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x01, C_ALL, "cycle_activity.cycles_l2_pending_core"             , 0x0, ATTR_ANY , 0x0 }, \
{ 0xA3, 0x02, C0|C1|C2|C3, "cycle_activity.cycles_ldm_pending"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x02, C0|C1|C2|C3, "cycle_activity.cycles_ldm_pending_core"      , 0x0, ATTR_ANY , 0x0 }, \
{ 0xA3, 0x08, C(2), "cycle_activity.cycles_l1d_pending"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x08, C(2), "cycle_activity.cycles_l1d_pending_core"             , 0x0, ATTR_ANY , 0x0 }, \
{ 0xA3, 0x04, C_ALL, "cycle_activity.cycles_no_execute"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x04, C_ALL, "cycle_activity.cycles_no_execute_core"             , 0x0, ATTR_ANY , 0x0 }, \
{ 0xAB, 0x01, C_ALL, "dsb2mite_switches.count"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xAB, 0x02, C_ALL, "dsb2mite_switches.penalty_cycles"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xAC, 0x08, C_ALL, "dsb_fill.exceed_dsb_lines"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xAE, 0x01, C_ALL, "itlb.itlb_flush"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x01, C_ALL, "offcore_requests.demand_data_rd"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x02, C_ALL, "offcore_requests.demand_code_rd"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x04, C_ALL, "offcore_requests.demand_rfo"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x08, C_ALL, "offcore_requests.all_data_rd"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x01, C_ALL, "uops_executed.thread"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x01, C_ALL, "uops_executed.stall_cycles"                        , 0x1, ATTR_INV , 0x0 }, \
{ 0xB1, 0x02, C_ALL, "uops_executed.core"                                , 0x0, ATTR_NONE, 0x0 }, \
/* { 0xB7, 0x01, C_ALL, "offcore_response_0"                             , 0x0, ATTR_NONE, 0x1A6 }, ignore events that require msr_offset */ \
/* { 0xBB, 0x01, C_ALL, "offcore_response_1"                             , 0x0, ATTR_NONE, 0x1A7 }, ignore events that require msr_offset */ \
{ 0xBD, 0x01, C_ALL, "tlb_flush.dtlb_thread"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xBD, 0x20, C_ALL, "tlb_flush.stlb_any"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC0, 0x00, C_ALL, "inst_retired.any_p"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC0, 0x01, C(1), "inst_retired.prec_dist"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC1, 0x08, C_ALL, "other_assists.avx_store"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC1, 0x10, C_ALL, "other_assists.avx_to_sse"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC1, 0x20, C_ALL, "other_assists.sse_to_avx"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC2, 0x01, C_ALL, "uops_retired.all"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC2, 0x01, C_ALL, "uops_retired.active_cycles"                        , 0x1, ATTR_NONE, 0x0 }, \
{ 0xC2, 0x01, C_ALL, "uops_retired.stall_cycles"                         , 0x1, ATTR_INV , 0x0 }, \
{ 0xC2, 0x02, C_ALL, "uops_retired.retire_slots"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC3, 0x02, C_ALL, "machine_clears.memory_ordering"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC3, 0x04, C_ALL, "machine_clears.smc"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC3, 0x20, C_ALL, "machine_clears.maskmov"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x00, C_ALL, "br_inst_retired.all_branches"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x01, C_ALL, "br_inst_retired.conditional"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x02, C_ALL, "br_inst_retired.near_call"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x04, C_ALL, "br_inst_retired.all_branches"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x08, C_ALL, "br_inst_retired.near_return"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x10, C_ALL, "br_inst_retired.not_taken"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x20, C_ALL, "br_inst_retired.near_taken"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x40, C_ALL, "br_inst_retired.far_branch"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC5, 0x00, C_ALL, "br_misp_retired.all_branches"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC5, 0x01, C_ALL, "br_misp_retired.conditional"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC5, 0x02, C_ALL, "br_misp_retired.near_call"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC5, 0x04, C_ALL, "br_misp_retired.all_branches"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC5, 0x10, C_ALL, "br_misp_retired.not_taken"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC5, 0x20, C_ALL, "br_misp_retired.taken"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCA, 0x02, C_ALL, "fp_assist.x87_output"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCA, 0x04, C_ALL, "fp_assist.x87_input"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCA, 0x08, C_ALL, "fp_assist.simd_output"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCA, 0x10, C_ALL, "fp_assist.simd_input"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCA, 0x1E, C_ALL, "fp_assist.any"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCC, 0x20, C_ALL, "rob_misc_events.lbr_inserts"                       , 0x0, ATTR_NONE, 0x0 }, \
/* { 0xCD, 0x01, C3   , "mem_trans_retired.load_latency"                    , 0x0, ATTR_NONE, 0x3F6 }, ignore events that require msr_offset */ /* See Section "MSR_PEBS_LD_LAT_THRESHOLD" */ \
{ 0xCD, 0x02, C3   , "mem_trans_retired.precise_store"                   , 0x0, ATTR_NONE, 0x0 }, /* See Section "Precise Store Facility" */ \
{ 0xD0, 0x11, C_ALL, "mem_uops_retired.stlb_miss_loads"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD0, 0x12, C_ALL, "mem_uops_retired.stlb_miss_stores"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD0, 0x21, C_ALL, "mem_uops_retired.lock_loads"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD0, 0x22, C_ALL, "mem_uops_retired.lock_stores"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD0, 0x41, C_ALL, "mem_uops_retired.split_loads"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD0, 0x42, C_ALL, "mem_uops_retired.split_stores"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD0, 0x81, C_ALL, "mem_uops_retired.all_loads"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD0, 0x82, C_ALL, "mem_uops_retired.all_stores"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD1, 0x01, C_ALL, "mem_load_uops_retired.l1_hit"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD1, 0x02, C_ALL, "mem_load_uops_retired.l2_hit"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD1, 0x04, C_ALL, "mem_load_uops_retired.llc_hit"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD1, 0x08, C_ALL, "mem_load_uops_retired.l1_miss"                     , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD1, 0x10, C_ALL, "mem_load_uops_retired.l2_miss"                     , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD1, 0x20, C_ALL, "mem_load_uops_retired.llc_miss"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD1, 0x40, C_ALL, "mem_load_uops_retired.hit_lfb"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD2, 0x01, C_ALL, "mem_load_uops_llc_hit_retired.xsnp_miss"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD2, 0x02, C_ALL, "mem_load_uops_llc_hit_retired.xsnp_hit"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD2, 0x04, C_ALL, "mem_load_uops_llc_hit_retired.xsnp_hitm"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD2, 0x08, C_ALL, "mem_load_uops_llc_hit_retired.xsnp_none"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD3, 0x01, C_ALL, "mem_load_uops_llc_miss_retired.local_dram"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xE6, 0x1F, C_ALL, "baclears.any"                                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x01, C_ALL, "l2_trans.demand_data_rd"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x02, C_ALL, "l2_trans.rfo"                                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x04, C_ALL, "l2_trans.code_rd"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x08, C_ALL, "l2_trans.all_pf"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x10, C_ALL, "l2_trans.l1d_wb"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x20, C_ALL, "l2_trans.l2_fill"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x40, C_ALL, "l2_trans.l2_wb"                                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x80, C_ALL, "l2_trans.all_requests"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF1, 0x01, C_ALL, "l2_lines_in.i"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF1, 0x02, C_ALL, "l2_lines_in.s"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF1, 0x04, C_ALL, "l2_lines_in.e"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF1, 0x07, C_ALL, "l2_lines_in.all"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x01, C_ALL, "l2_lines_out.demand_clean"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x02, C_ALL, "l2_lines_out.demand_dirty"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x04, C_ALL, "l2_lines_out.pf_clean"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x08, C_ALL, "l2_lines_out.pf_dirty"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x0A, C_ALL, "l2_lines_out.dirty_all"                            , 0x0, ATTR_NONE, 0x0 }, \
/* end of #define */

#define EVENTS_FAM6_MOD62_ONLY \
{ 0xD3, 0x01, C_ALL, "mem_load_uops_llc_miss_retired.local_dram"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD3, 0x04, C_ALL, "mem_load_uops_llc_miss_retired.remote_dram"        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD3, 0x10, C_ALL, "mem_load_uops_llc_miss_retired.remote_hitm"        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD3, 0x20, C_ALL, "mem_load_uops_llc_miss_retired.remote_fwd"         , 0x0, ATTR_NONE, 0x0 }, \
/* end of #define */

/* Intel Haswell Processor */
/*
 * The Haswell tables take into account Bug 17006019
 *   libcpc counter names should be based on public Intel documentation -- Haswell
 * and are basically from the
 * Intel SDM, June 2013, Section 19.3, Table 19-2 and Table 19-3.
 * We omit the Table 19-4 uncore events.
 */

#define EVENTS_FAM6_MOD60 \
{ 0x03, 0x02, C_ALL, "ld_blocks.store_forward"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x03, 0x08, C_ALL, "ld_blocks.no_sr"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x05, 0x01, C_ALL, "misalign_mem_ref.loads"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x05, 0x02, C_ALL, "misalign_mem_ref.stores"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x07, 0x01, C_ALL, "ld_blocks_partial.address_alias"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x01, C_ALL, "dtlb_load_misses.miss_causes_a_walk"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x02, C_ALL, "dtlb_load_misses.walk_completed_4k"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x04, C_ALL, "dtlb_load_misses.walk_completed_2m_4m"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x0E, C_ALL, "dtlb_load_misses.walk_completed"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x10, C_ALL, "dtlb_load_misses.walk_duration"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x20, C_ALL, "dtlb_load_misses.stlb_hit_4k"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x40, C_ALL, "dtlb_load_misses.stlb_hit_2m"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x60, C_ALL, "dtlb_load_misses.stlb_hit"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x80, C_ALL, "dtlb_load_misses.pde_cache_miss"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0D, 0x03, C_ALL, "int_misc.recovery_cycles"                          , 0x1, ATTR_NONE, 0x0 }, \
{ 0x0D, 0x03, C_ALL, "int_misc.recovery_cycles_occurrences"              , 0x1, ATTR_EDGE, 0x0 }, \
{ 0x0E, 0x01, C_ALL, "uops_issued.any"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0E, 0x01, C_ALL, "uops_issued.stall_cycles"                          , 0x1, ATTR_INV , 0x0 }, \
{ 0x0E, 0x01, C_ALL, "uops_issued.core_stall_cycles"                     , 0x1, ATTR_INV | ATTR_ANY, 0x0 }, \
{ 0x0E, 0x10, C_ALL, "uops_issued.flags_merge"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0E, 0x20, C_ALL, "uops_issued.slow_lea"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0E, 0x40, C_ALL, "uops_issued.single_mul"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x21, C_ALL, "l2_rqsts.demand_data_rd_miss"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x22, C_ALL, "l2_rqsts.rfo_miss"                                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x24, C_ALL, "l2_rqsts.code_rd_miss"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x27, C_ALL, "l2_rqsts.all_demand_miss"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x30, C_ALL, "l2_rqsts.l2_pf_miss"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x3F, C_ALL, "l2_rqsts.miss"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x41, C_ALL, "l2_rqsts.demand_data_rd_hit"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x42, C_ALL, "l2_rqsts.rfo_hit"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x44, C_ALL, "l2_rqsts.code_rd_hit"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x50, C_ALL, "l2_rqsts.l2_pf_hit"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xE1, C_ALL, "l2_rqsts.all_demand_data_rd"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xE2, C_ALL, "l2_rqsts.all_rfo"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xE4, C_ALL, "l2_rqsts.all_code_rd"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xE7, C_ALL, "l2_rqsts.all_demand_references"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xF8, C_ALL, "l2_rqsts.all_pf"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xFF, C_ALL, "l2_rqsts.references"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x27, 0x50, C_ALL, "l2_demand_rqsts.wb_hit"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2E, 0x4F, C_ALL, "longest_lat_cache.reference"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2E, 0x41, C_ALL, "longest_lat_cache.miss"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3C, 0x00, C_ALL, "cpu_clk_unhalted.thread_p"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3C, 0x01, C_ALL, "cpu_clk_thread_unhalted.ref_xclk"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x48, 0x01, C(2) , "l1d_pend_miss.pending"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x48, 0x01, C(2) , "l1d_pend_miss.pending_cycles"                      , 0x1, ATTR_NONE, 0x0 }, \
{ 0x48, 0x01, C(2) , "l1d_pend_miss.occurences"                          , 0x1, ATTR_EDGE, 0x0 }, \
{ 0x49, 0x01, C_ALL, "dtlb_store_misses.miss_causes_a_walk"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x02, C_ALL, "dtlb_store_misses.walk_completed_4k"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x04, C_ALL, "dtlb_store_misses.walk_completed_2m_4m"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x0E, C_ALL, "dtlb_store_misses.walk_completed"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x10, C_ALL, "dtlb_store_misses.walk_duration"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x20, C_ALL, "dtlb_store_misses.stlb_hit_4k"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x40, C_ALL, "dtlb_store_misses.stlb_hit_2m"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x60, C_ALL, "dtlb_store_misses.stlb_hit"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x80, C_ALL, "dtlb_store_misses.pde_cache_miss"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4C, 0x01, C_ALL, "load_hit_pre.sw_pf"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4C, 0x02, C_ALL, "load_hit_pre.hw_pf"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x51, 0x01, C_ALL, "l1d.replacement"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x54, 0x01, C_ALL, "tx_mem.abort_conflict"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x54, 0x02, C_ALL, "tx_mem.abort_capacity"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x54, 0x04, C_ALL, "tx_mem.abort_hle_store_to_elided_lock"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x54, 0x08, C_ALL, "tx_mem.abort_hle_elision_buffer_not_empty"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x54, 0x10, C_ALL, "tx_mem.abort_hle_elision_buffer_mismatch"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x54, 0x20, C_ALL, "tx_mem.abort_hle_elision_buffer_unsupported_alignment"  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x54, 0x40, C_ALL, "tx_mem.abort_hle_elision_buffer_full"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x58, 0x01, C_ALL, "move_elimination.int_eliminated"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x58, 0x02, C_ALL, "move_elimination.simd_eliminated"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x58, 0x04, C_ALL, "move_elimination.int_not_eliminated"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x58, 0x08, C_ALL, "move_elimination.simd_not_eliminated"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x5C, 0x01, C_ALL, "cpl_cycles.ring0"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x5C, 0x01, C_ALL, "cpl_cycles.ring0_trans"                            , 0x0, ATTR_EDGE, 0x0 }, \
{ 0x5C, 0x02, C_ALL, "cpl_cycles.ring123"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x5D, 0x01, C_ALL, "tx_exec.misc1"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x5D, 0x02, C_ALL, "tx_exec.misc2"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x5D, 0x04, C_ALL, "tx_exec.misc3"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x5D, 0x08, C_ALL, "tx_exec.misc4"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x5D, 0x10, C_ALL, "tx_exec.misc5"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x5E, 0x01, C_ALL, "rs_events.empty_cycles"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x01, C_ALL, "offcore_requests_outstanding.demand_data_rd"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x01, C_ALL, "offcore_requests_outstanding.cycles_with_demand_data_rd", 0x1, ATTR_NONE, 0x0 }, \
{ 0x60, 0x02, C_ALL, "offcore_requests_outstanding.demand_code_rd"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x02, C_ALL, "offcore_requests_outstanding.demand_code_rd_cycles", 0x1, ATTR_NONE, 0x0 }, \
{ 0x60, 0x04, C_ALL, "offcore_requests_outstanding.demand_rfo"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x04, C_ALL, "offcore_requests_outstanding.demand_rfo_cycles"    , 0x1, ATTR_NONE, 0x0 }, \
{ 0x60, 0x08, C_ALL, "offcore_requests_outstanding.all_data_rd"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x08, C_ALL, "offcore_requests_outstanding.cycles_with_data_rd"  , 0x1, ATTR_NONE, 0x0 }, \
{ 0x63, 0x01, C_ALL, "lock_cycles.split_lock_uc_lock_duration"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x63, 0x02, C_ALL, "lock_cycles.cache_lock_duration"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x02, C_ALL, "idq.empty"                                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x04, C_ALL, "idq.mite_uops"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x04, C_ALL, "idq.mite_cycles"                                   , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x20, C_ALL, "idq.ms_mite_uops"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x20, C_ALL, "idq.ms_mite_cycles"                                , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x24, C_ALL, "idq.all_mite_cycles_any_uops"                      , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x24, C_ALL, "idq.all_mite_cycles_4_uops"                        , 0x4, ATTR_NONE, 0x0 }, \
{ 0x79, 0x08, C_ALL, "idq.dsb_uops"                                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x08, C_ALL, "idq.dsb_cycles"                                    , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x10, C_ALL, "idq.ms_dsb_uops"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x10, C_ALL, "idq.ms_dsb_cycles"                                 , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x10, C_ALL, "idq.ms_dsb_occur"                                  , 0x1, ATTR_EDGE, 0x0 }, \
{ 0x79, 0x18, C_ALL, "idq.all_dsb_cycles_any_uops"                       , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x18, C_ALL, "idq.all_dsb_cycles_4_uops"                         , 0x4, ATTR_NONE, 0x0 }, \
{ 0x79, 0x30, C_ALL, "idq.ms_uops"                                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x30, C_ALL, "idq.ms_cycles"                                     , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x3C, C_ALL, "idq.mite_all_uops"                                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x80, 0x02, C_ALL, "icache.misses"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x01, C_ALL, "itlb_misses.miss_causes_a_walk"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x02, C_ALL, "itlb_misses.walk_completed_4k"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x04, C_ALL, "itlb_misses.walk_completed_2m_4m"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x0E, C_ALL, "itlb_misses.walk_completed"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x10, C_ALL, "itlb_misses.walk_duration"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x20, C_ALL, "itlb_misses.stlb_hit_4k"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x40, C_ALL, "itlb_misses.stlb_hit_2m"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x60, C_ALL, "itlb_misses.stlb_hit"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x87, 0x01, C_ALL, "ild_stall.lcp"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x87, 0x04, C_ALL, "ild_stall.iq_full"                                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x41, C_ALL, "br_inst_exec.nontaken_conditional"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x81, C_ALL, "br_inst_exec.taken_conditional"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x82, C_ALL, "br_inst_exec.taken_direct_jump"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x84, C_ALL, "br_inst_exec.taken_indirect_jump_non_call_ret"     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x88, C_ALL, "br_inst_exec.taken_indirect_near_return"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x90, C_ALL, "br_inst_exec.taken_direct_near_call"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0xA0, C_ALL, "br_inst_exec.taken_indirect_near_call"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0xFF, C_ALL, "br_inst_exec.all_branches"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x41, C_ALL, "br_misp_exec.nontaken_conditional"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x81, C_ALL, "br_misp_exec.taken_conditional"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x84, C_ALL, "br_misp_exec.taken_indirect_jump_non_call_ret"     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x88, C_ALL, "br_misp_exec.taken_return_near"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x90, C_ALL, "br_misp_exec.taken_direct_near_call"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0xA0, C_ALL, "br_misp_exec.taken_indirect_near_call"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0xFF, C_ALL, "br_misp_exec.all_branches"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x9C, 0x01, C_ALL, "idq_uops_not_delivered.core"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x9C, 0x01, C_ALL, "idq_uops_not_delivered.cycles_0_uops_deliv.core"   , 0x4, ATTR_NONE, 0x0 }, \
{ 0x9C, 0x01, C_ALL, "idq_uops_not_delivered.cycles_le_1_uop_deliv.core" , 0x3, ATTR_NONE, 0x0 }, \
{ 0x9C, 0x01, C_ALL, "idq_uops_not_delivered.cycles_le_2_uop_deliv.core" , 0x2, ATTR_NONE, 0x0 }, \
{ 0x9C, 0x01, C_ALL, "idq_uops_not_delivered.cycles_le_3_uop_deliv.core" , 0x1, ATTR_NONE, 0x0 }, \
{ 0x9C, 0x01, C_ALL, "idq_uops_not_delivered.cycles_fe_was_ok"           , 0x1, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x01, C_ALL, "uops_executed_port.port_0"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x02, C_ALL, "uops_executed_port.port_1"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x04, C_ALL, "uops_executed_port.port_2"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x08, C_ALL, "uops_executed_port.port_3"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x10, C_ALL, "uops_executed_port.port_4"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x20, C_ALL, "uops_executed_port.port_5"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x40, C_ALL, "uops_executed_port.port_6"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x80, C_ALL, "uops_executed_port.port_7"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x01, C_ALL, "uops_executed_port.port_0_core"                    , 0x0, ATTR_ANY , 0x0 }, \
{ 0xA1, 0x02, C_ALL, "uops_executed_port.port_1_core"                    , 0x0, ATTR_ANY , 0x0 }, \
{ 0xA1, 0x04, C_ALL, "uops_executed_port.port_2_core"                    , 0x0, ATTR_ANY , 0x0 }, \
{ 0xA1, 0x08, C_ALL, "uops_executed_port.port_3_core"                    , 0x0, ATTR_ANY , 0x0 }, \
{ 0xA1, 0x10, C_ALL, "uops_executed_port.port_4_core"                    , 0x0, ATTR_ANY , 0x0 }, \
{ 0xA1, 0x20, C_ALL, "uops_executed_port.port_5_core"                    , 0x0, ATTR_ANY , 0x0 }, \
{ 0xA1, 0x40, C_ALL, "uops_executed_port.port_6_core"                    , 0x0, ATTR_ANY , 0x0 }, \
{ 0xA1, 0x80, C_ALL, "uops_executed_port.port_7_core"                    , 0x0, ATTR_ANY , 0x0 }, \
{ 0xA2, 0x01, C_ALL, "resource_stalls.any"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x04, C_ALL, "resource_stalls.rs"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x08, C_ALL, "resource_stalls.sb"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x10, C_ALL, "resource_stalls.rob"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x01, C_ALL, "cycle_activity.cycles_l2_pending"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x01, C_ALL, "cycle_activity.cycles_l2_pending_cycles"           , 0x2, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x02, C_ALL, "cycle_activity.cycles_ldm_pending"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x02, C_ALL, "cycle_activity.cycles_ldm_pending_cycles"          , 0x2, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x05, C_ALL, "cycle_activity.stalls_l2_pending"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x08, C(2) , "cycle_activity.cycles_l1d_pending"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x08, C(2) , "cycle_activity.cycles_l1d_pending_cycles"          , 0x8, ATTR_NONE, 0x0 }, \
{ 0xA8, 0x01, C_ALL, "lsd.uops"                                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xAE, 0x01, C_ALL, "itlb.itlb_flush"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x01, C_ALL, "offcore_requests.demand_data_rd"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x02, C_ALL, "offcore_requests.demand_code_rd"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x04, C_ALL, "offcore_requests.demand_rfo"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x08, C_ALL, "offcore_requests.all_data_rd"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x02, C_ALL, "uops_executed.core"                                , 0x0, ATTR_NONE, 0x0 }, \
/* { 0xB7, 0x01, C_ALL, "off_core_response_0"                               , 0x0, ATTR_NONE, 0x1A6 }, omit events requiring MSR programming */ \
/* { 0xBB, 0x01, C_ALL, "off_core_response_1"                               , 0x0, ATTR_NONE, 0x1A7 }, omit events requiring MSR programming */ \
{ 0xBC, 0x11, C_ALL, "page_walker_loads.dtlb_l1"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xBC, 0x21, C_ALL, "page_walker_loads.itlb_l1"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xBC, 0x12, C_ALL, "page_walker_loads.dtlb_l2"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xBC, 0x22, C_ALL, "page_walker_loads.itlb_l2"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xBC, 0x14, C_ALL, "page_walker_loads.dtlb_l3"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xBC, 0x24, C_ALL, "page_walker_loads.itlb_l3"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xBC, 0x18, C_ALL, "page_walker_loads.dtlb_memory"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xBC, 0x28, C_ALL, "page_walker_loads.itlb_memory"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xBD, 0x01, C_ALL, "tlb_flush.dtlb_thread"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xBD, 0x20, C_ALL, "tlb_flush.stlb_any"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC0, 0x00, C_ALL, "inst_retired.any_p"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC0, 0x01, C(1) , "inst_retired.prec_dist"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC1, 0x08, C_ALL, "other_assists.avx_to_sse"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC1, 0x10, C_ALL, "other_assists.sse_to_avx"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC1, 0x40, C_ALL, "other_assists.any_wb_assist"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC2, 0x01, C_ALL, "uops_retired.all"                                  , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC2, 0x01, C_ALL, "uops_retired.stall_cycles"                         , 0x1, ATTR_INV , 0x0 }, \
{ 0xC2, 0x02, C_ALL, "uops_retired.retire_slots"                         , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC3, 0x02, C_ALL, "machine_clears.memory_ordering"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC3, 0x04, C_ALL, "machine_clears.smc"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC3, 0x20, C_ALL, "machine_clears.maskmov"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x00, C_ALL, "br_inst_retired.all_branches"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x01, C_ALL, "br_inst_retired.conditional"                       , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC4, 0x02, C_ALL, "br_inst_retired.near_call"                         , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC4, 0x04, C_ALL, "br_inst_retired.all_branches"                      , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC4, 0x08, C_ALL, "br_inst_retired.near_return"                       , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC4, 0x10, C_ALL, "br_inst_retired.not_taken"                         , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC4, 0x20, C_ALL, "br_inst_retired.near_taken"                        , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC4, 0x40, C_ALL, "br_inst_retired.far_branch"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC5, 0x00, C_ALL, "br_misp_retired.all_branches"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC5, 0x01, C_ALL, "br_misp_retired.conditional"                       , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC5, 0x04, C_ALL, "br_misp_retired.all_branches"                      , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC5, 0x20, C_ALL, "br_misp_retired.near_taken"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC8, 0x01, C_ALL, "hle_retired.start"                                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC8, 0x02, C_ALL, "hle_retired.commit"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC8, 0x04, C_ALL, "hle_retired.aborted"                               , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC8, 0x08, C_ALL, "hle_retired.aborted_misc1"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC8, 0x10, C_ALL, "hle_retired.aborted_misc2"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC8, 0x20, C_ALL, "hle_retired.aborted_misc3"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC8, 0x40, C_ALL, "hle_retired.aborted_misc4"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC8, 0x80, C_ALL, "hle_retired.aborted_misc5"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC9, 0x01, C_ALL, "rtm_retired.start"                                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC9, 0x02, C_ALL, "rtm_retired.commit"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC9, 0x04, C_ALL, "rtm_retired.aborted"                               , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC9, 0x08, C_ALL, "rtm_retired.aborted_misc1"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC9, 0x10, C_ALL, "rtm_retired.aborted_misc2"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC9, 0x20, C_ALL, "rtm_retired.aborted_misc3"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC9, 0x40, C_ALL, "rtm_retired.aborted_misc4"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC9, 0x80, C_ALL, "rtm_retired.aborted_misc5"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCA, 0x02, C_ALL, "fp_assist.x87_output"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCA, 0x04, C_ALL, "fp_assist.x87_input"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCA, 0x08, C_ALL, "fp_assist.simd_output"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCA, 0x10, C_ALL, "fp_assist.simd_input"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCA, 0x1E, C_ALL, "fp_assist.any"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCC, 0x20, C_ALL, "rob_misc_events.lbr_inserts"                       , 0x0, ATTR_NONE, 0x0 }, \
/* { 0xCD, 0x01, C_ALL, "mem_trans_retired.load_latency"                    , 0x0, ATTR_NONE, 0x3F6 }, omit events requiring MSR programming */ \
{ 0xD0, 0x11, C_ALL, "mem_uops_retired.stlb_miss_loads"                  , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD0, 0x12, C_ALL, "mem_uops_retired.stlb_miss_stores"                 , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD0, 0x21, C_ALL, "mem_uops_retired.lock_loads"                       , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD0, 0x22, C_ALL, "mem_uops_retired.lock_stores"                      , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD0, 0x41, C_ALL, "mem_uops_retired.split_loads"                      , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD0, 0x42, C_ALL, "mem_uops_retired.split_stores"                     , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD0, 0x81, C_ALL, "mem_uops_retired.all_loads"                        , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD0, 0x82, C_ALL, "mem_uops_retired.all_stores"                       , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD1, 0x01, C_ALL, "mem_load_uops_retired.l1_hit"                      , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD1, 0x02, C_ALL, "mem_load_uops_retired.l2_hit"                      , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD1, 0x04, C_ALL, "mem_load_uops_retired.l3_hit"                      , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD1, 0x08, C_ALL, "mem_load_uops_retired.l1_miss"                     , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD1, 0x10, C_ALL, "mem_load_uops_retired.l2_miss"                     , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD1, 0x20, C_ALL, "mem_load_uops_retired.l3_miss"                     , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD1, 0x40, C_ALL, "mem_load_uops_retired.hit_lfb"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xD2, 0x01, C_ALL, "mem_load_uops_l3_hit_retired.xsnp_miss"            , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD2, 0x02, C_ALL, "mem_load_uops_l3_hit_retired.xsnp_hit"             , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD2, 0x04, C_ALL, "mem_load_uops_l3_hit_retired.xsnp_hitm"            , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD2, 0x08, C_ALL, "mem_load_uops_l3_hit_retired.xsnp_none"            , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD3, 0x01, C_ALL, "mem_load_uops_l3_miss_retired.local_dram"          , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xE6, 0x1F, C_ALL, "baclears.any"                                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x01, C_ALL, "l2_trans.demand_data_rd"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x02, C_ALL, "l2_trans.rfo"                                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x04, C_ALL, "l2_trans.code_rd"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x08, C_ALL, "l2_trans.all_pf"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x10, C_ALL, "l2_trans.l1d_wb"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x20, C_ALL, "l2_trans.l2_fill"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x40, C_ALL, "l2_trans.l2_wb"                                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x80, C_ALL, "l2_trans.all_requests"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF1, 0x01, C_ALL, "l2_lines_in.i"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF1, 0x02, C_ALL, "l2_lines_in.s"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF1, 0x04, C_ALL, "l2_lines_in.e"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF1, 0x07, C_ALL, "l2_lines_in.all"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x05, C_ALL, "l2_lines_out.demand_clean"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x06, C_ALL, "l2_lines_out.demand_dirty"                         , 0x0, ATTR_NONE, 0x0 }, \
/* end of #define */

/* Intel Broadwell Processor */
/*
 * This table is essentially taken from:
 *   https://grok.cz.oracle.com/source/xref/on12-clone/usr/src/uts/intel/pcbe/bdw_pcbe_tbl.c
 */

#define EVENTS_FAM6_MOD61 \
{ 0x03, 0x02, C_ALL, "ld_blocks.store_forward"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x03, 0x08, C_ALL, "ld_blocks.no_sr"                                   , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0x05, 0x01, C_ALL, "misalign_mem_ref.loads"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x05, 0x02, C_ALL, "misalign_mem_ref.stores"                           , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0x07, 0x01, C_ALL, "ld_blocks_partial.address_alias"                   , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0x08, 0x01, C_ALL, "dtlb_load_misses.miss_causes_a_walk"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x02, C_ALL, "dtlb_load_misses.walk_completed_4k"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x04, C_ALL, "dtlb_load_misses.walk_completed_2m_4m"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x0E, C_ALL, "dtlb_load_misses.walk_completed"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x10, C_ALL, "dtlb_load_misses.walk_duration"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x20, C_ALL, "dtlb_load_misses.stlb_hit_4k"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x40, C_ALL, "dtlb_load_misses.stlb_hit_2m"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x60, C_ALL, "dtlb_load_misses.stlb_hit"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x80, C_ALL, "dtlb_load_misses.pde_cache_miss"                   , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0x0D, 0x03, C_ALL, "int_misc.recovery_cycles"                          , 0x1, ATTR_NONE, 0x0 }, \
{ 0x0D, 0x03, C_ALL, "int_misc.recovery_cycles_any"                      , 0x1, ATTR_ANY , 0x0 }, \
/* Private event, not public by Intel */ \
{ 0x0D, 0x03, C_ALL, "int_misc.recovery_cycles_occurrences"              , 0x1, ATTR_EDGE, 0x0 }, \
{ 0x0D, 0x08, C_ALL, "int_misc.rat_stall_cycles"                         , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0x0E, 0x01, C_ALL, "uops_issued.any"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0E, 0x10, C_ALL, "uops_issued.flags_merge"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0E, 0x20, C_ALL, "uops_issued.slow_lea"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0E, 0x40, C_ALL, "uops_issued.single_mul"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0E, 0x01, C_ALL, "uops_issued.stall_cycles"                          , 0x1, ATTR_INV , 0x0 }, \
{ 0x0E, 0x01, C_ALL, "uops_issued.core_stall_cycles"                     , 0x1,(ATTR_INV | ATTR_ANY), 0x0 }, \
 \
{ 0x14, 0x01, C_ALL, "arith.fpu_div_active"                              , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0x24, 0x21, C_ALL, "l2_rqsts.demand_data_rd_miss"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x41, C_ALL, "l2_rqsts.demand_data_rd_hit"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x30, C_ALL, "l2_rqsts.l2_pf_miss"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x50, C_ALL, "l2_rqsts.l2_pf_hit"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xE1, C_ALL, "l2_rqsts.all_demand_data_rd"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xE2, C_ALL, "l2_rqsts.all_rfo"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xE4, C_ALL, "l2_rqsts.all_code_rd"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xF8, C_ALL, "l2_rqsts.all_pf"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x42, C_ALL, "l2_rqsts.rfo_hit"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x22, C_ALL, "l2_rqsts.rfo_miss"                                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x44, C_ALL, "l2_rqsts.code_rd_hit"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x24, C_ALL, "l2_rqsts.code_rd_miss"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x27, C_ALL, "l2_rqsts.all_demand_miss"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xE7, C_ALL, "l2_rqsts.all_demand_references"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x3F, C_ALL, "l2_rqsts.miss"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xFF, C_ALL, "l2_rqsts.references"                               , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0x27, 0x50, C_ALL, "l2_demand_rqsts.wb_hit"                            , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0x3C, 0x00, C_ALL, "cpu_clk_unhalted.thread_p"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3C, 0x00, C_ALL, "cpu_clk_unhalted.thread_p_any"                     , 0x0, ATTR_ANY , 0x0 }, \
{ 0x3C, 0x01, C_ALL, "cpu_clk_thread_unhalted.ref_xclk"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3C, 0x01, C_ALL, "cpu_clk_thread_unhalted.ref_xclk_any"              , 0x0, ATTR_ANY , 0x0 }, \
{ 0x3C, 0x02, C_ALL, "cpu_clk_thread_unhalted.one_thread_active"         , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0x48, 0x01, C(2) , "l1d_pend_miss.pending"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x48, 0x01, C(2) , "l1d_pend_miss.pending_cycles"                      , 0x1, ATTR_NONE, 0x0 }, \
{ 0x48, 0x01, C(2) , "l1d_pend_miss.pending_cycles_any"                  , 0x1, ATTR_ANY , 0x0 }, \
/* Private event, not public by Intel */ \
{ 0x48, 0x01, C(2) , "l1d_pend_miss.occurences"                          , 0x1, ATTR_EDGE, 0x0 }, \
{ 0x48, 0x02, C_ALL, "l1d_pend_miss.fb_full"                             , 0x1, ATTR_NONE, 0x0 }, \
 \
{ 0x49, 0x01, C_ALL, "dtlb_store_misses.miss_causes_a_walk"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x02, C_ALL, "dtlb_store_misses.walk_completed_4k"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x04, C_ALL, "dtlb_store_misses.walk_completed_2m_4m"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x0E, C_ALL, "dtlb_store_misses.walk_completed"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x10, C_ALL, "dtlb_store_misses.walk_duration"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x20, C_ALL, "dtlb_store_misses.stlb_hit_4k"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x40, C_ALL, "dtlb_store_misses.stlb_hit_2m"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x60, C_ALL, "dtlb_store_misses.stlb_hit"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x80, C_ALL, "dtlb_store_misses.pde_cache_miss"                  , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0x4C, 0x01, C_ALL, "load_hit_pre.sw_pf"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4C, 0x02, C_ALL, "load_hit_pre.hw_pf"                                , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0x4F, 0x10, C_ALL, "ept.walk_cycles"                                   , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0x51, 0x01, C_ALL, "l1d.replacement"                                   , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0x54, 0x01, C_ALL, "tx_mem.abort_conflict"                             , 0x0, ATTR_TSX , 0x0 }, \
{ 0x54, 0x02, C_ALL, "tx_mem.abort_capacity_write"                       , 0x0, ATTR_TSX , 0x0 }, \
{ 0x54, 0x04, C_ALL, "tx_mem.abort_hle_store_to_elided_lock"             , 0x0, ATTR_TSX , 0x0 }, \
{ 0x54, 0x08, C_ALL, "tx_mem.abort_hle_elision_buffer_not_empty"         , 0x0, ATTR_TSX , 0x0 }, \
{ 0x54, 0x10, C_ALL, "tx_mem.abort_hle_elision_buffer_mismatch"          , 0x0, ATTR_TSX , 0x0 }, \
{ 0x54, 0x20, C_ALL, "tx_mem.abort_hle_elision_buffer_unsupported_alignment"  , 0x0, ATTR_TSX , 0x0 }, \
{ 0x54, 0x40, C_ALL, "tx_mem.hle_elision_buffer_full"                    , 0x0, ATTR_TSX , 0x0 }, \
 \
{ 0x58, 0x01, C_ALL, "move_elimination.int_eliminated"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x58, 0x02, C_ALL, "move_elimination.simd_eliminated"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x58, 0x04, C_ALL, "move_elimination.int_not_eliminated"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x58, 0x08, C_ALL, "move_elimination.simd_not_eliminated"              , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0x5C, 0x01, C_ALL, "cpl_cycles.ring0"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x5C, 0x01, C_ALL, "cpl_cycles.ring0_trans"                            , 0x1, ATTR_EDGE, 0x0 }, \
{ 0x5C, 0x02, C_ALL, "cpl_cycles.ring123"                                , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0x5D, 0x01, C_ALL, "tx_exec.misc1"                                     , 0x0, ATTR_TSX , 0x0 }, \
{ 0x5D, 0x02, C_ALL, "tx_exec.misc2"                                     , 0x0, ATTR_TSX , 0x0 }, \
{ 0x5D, 0x04, C_ALL, "tx_exec.misc3"                                     , 0x0, ATTR_TSX , 0x0 }, \
{ 0x5D, 0x08, C_ALL, "tx_exec.misc4"                                     , 0x0, ATTR_TSX , 0x0 }, \
{ 0x5D, 0x10, C_ALL, "tx_exec.misc5"                                     , 0x0, ATTR_TSX , 0x0 }, \
 \
{ 0x5E, 0x01, C_ALL, "rs_events.empty_cycles"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x5E, 0x01, C_ALL, "rs_events.empty_end"                               , 0x1, (ATTR_INV | ATTR_EDGE), 0x0 }, \
 \
{ 0x60, 0x01, C_ALL, "offcore_requests_outstanding.demand_data_rd"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x01, C_ALL, "offcore_requests_outstanding.cycles_with_demand_data_rd", 0x1, ATTR_NONE, 0x0 }, \
{ 0x60, 0x01, C_ALL, "offcore_requests_outstanding.demand_data_rd_ge_6 " , 0x6, ATTR_NONE, 0x0 }, \
{ 0x60, 0x02, C_ALL, "offcore_requests_outstanding.demand_code_rd"       , 0x0, ATTR_NONE, 0x0 }, \
/* Private event, not public by Intel */ \
{ 0x60, 0x02, C_ALL, "offcore_requests_outstanding.demand_code_rd_cycles", 0x1, ATTR_NONE, 0x0 }, \
{ 0x60, 0x04, C_ALL, "offcore_requests_outstanding.demand_rfo"           , 0x0, ATTR_NONE, 0x0 }, \
/* Private event, not public by Intel */ \
{ 0x60, 0x04, C_ALL, "offcore_requests_outstanding.demand_rfo_cycles"    , 0x1, ATTR_NONE, 0x0 }, \
{ 0x60, 0x08, C_ALL, "offcore_requests_outstanding.all_data_rd"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x08, C_ALL, "offcore_requests_outstanding.cycles_with_data_rd"  , 0x1, ATTR_NONE, 0x0 }, \
 \
{ 0x63, 0x01, C_ALL, "lock_cycles.split_lock_uc_lock_duration"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x63, 0x02, C_ALL, "lock_cycles.cache_lock_duration"                   , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0x79, 0x02, C_ALL, "idq.empty"                                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x04, C_ALL, "idq.mite_uops"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x04, C_ALL, "idq.mite_cycles"                                   , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x08, C_ALL, "idq.dsb_uops"                                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x08, C_ALL, "idq.dsb_cycles"                                    , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x10, C_ALL, "idq.ms_dsb_uops"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x10, C_ALL, "idq.ms_dsb_cycles"                                 , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x10, C_ALL, "idq.ms_dsb_occur"                                  , 0x1, ATTR_EDGE, 0x0 }, \
{ 0x79, 0x18, C_ALL, "idq.all_dsb_cycles_4_uops"                         , 0x4, ATTR_NONE, 0x0 }, \
{ 0x79, 0x18, C_ALL, "idq.all_dsb_cycles_any_uops"                       , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x20, C_ALL, "idq.ms_mite_uops"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x24, C_ALL, "idq.all_mite_cycles_4_uops"                        , 0x4, ATTR_NONE, 0x0 }, \
{ 0x79, 0x24, C_ALL, "idq.all_mite_cycles_any_uops"                      , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x30, C_ALL, "idq.ms_uops"                                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x30, C_ALL, "idq.ms_cycles"                                     , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x30, C_ALL, "idq.ms_switches"                                   , 0x1, ATTR_EDGE, 0x0 }, \
{ 0x79, 0x3C, C_ALL, "idq.mite_all_uops"                                 , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0x80, 0x01, C_ALL, "icache.hit"                                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x80, 0x02, C_ALL, "icache.misses"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x80, 0x04, C_ALL, "icache.ifdata_stall"                               , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0x85, 0x01, C_ALL, "itlb_misses.miss_causes_a_walk"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x02, C_ALL, "itlb_misses.walk_completed_4k"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x04, C_ALL, "itlb_misses.walk_completed_2m_4m"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x0E, C_ALL, "itlb_misses.walk_completed"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x10, C_ALL, "itlb_misses.walk_duration"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x20, C_ALL, "itlb_misses.stlb_hit_4k"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x40, C_ALL, "itlb_misses.stlb_hit_2m"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x60, C_ALL, "itlb_misses.stlb_hit"                              , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0x87, 0x01, C_ALL, "ild_stall.lcp"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x87, 0x04, C_ALL, "ild_stall.iq_full"                                 , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0x88, 0x41, C_ALL, "br_inst_exec.nontaken_conditional"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x81, C_ALL, "br_inst_exec.taken_conditional"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x82, C_ALL, "br_inst_exec.taken_direct_jump"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x84, C_ALL, "br_inst_exec.taken_indirect_jump_non_call_ret"     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x88, C_ALL, "br_inst_exec.taken_indirect_near_return"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0x90, C_ALL, "br_inst_exec.taken_direct_near_call"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0xA0, C_ALL, "br_inst_exec.taken_indirect_near_call"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0xC1, C_ALL, "br_inst_exec.all_conditional"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0xC2, C_ALL, "br_inst_exec.all_direct_jmp"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0xC4, C_ALL, "br_inst_exec.all_indirect_jump_non_call_ret"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0xC8, C_ALL, "br_inst_exec.all_indirect_near_return"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0xD0, C_ALL, "br_inst_exec.all_direct_near_call"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x88, 0xFF, C_ALL, "br_inst_exec.all_branches"                         , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0x89, 0x41, C_ALL, "br_misp_exec.nontaken_conditional"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x81, C_ALL, "br_misp_exec.taken_conditional"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0x84, C_ALL, "br_misp_exec.taken_indirect_jump_non_call_ret"     , 0x0, ATTR_NONE, 0x0 }, \
/* Private event, not public by Intel */ \
{ 0x89, 0x88, C_ALL, "br_misp_exec.taken_return_near"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0xC1, C_ALL, "br_misp_exec.all_conditional"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0xC4, C_ALL, "br_misp_exec.all_indirect_jump_non_call_ret"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0xA0, C_ALL, "br_misp_exec.taken_indirect_near_call"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x89, 0xFF, C_ALL, "br_misp_exec.all_branches"                         , 0x0, ATTR_NONE, 0x0 }, \
 \
/* Use Cmask to qualify uop b/w */ \
{ 0x9C, 0x01, C_ALL, "idq_uops_not_delivered.core"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x9C, 0x01, C_ALL, "idq_uops_not_delivered.cycles_0_uops_deliv.core"   , 0x4, ATTR_NONE, 0x0 }, \
{ 0x9C, 0x01, C_ALL, "idq_uops_not_delivered.cycles_le_1_uop_deliv.core" , 0x3, ATTR_NONE, 0x0 }, \
{ 0x9C, 0x01, C_ALL, "idq_uops_not_delivered.cycles_le_2_uop_deliv.core" , 0x2, ATTR_NONE, 0x0 }, \
{ 0x9C, 0x01, C_ALL, "idq_uops_not_delivered.cycles_le_3_uop_deliv.core" , 0x1, ATTR_NONE, 0x0 }, \
{ 0x9C, 0x01, C_ALL, "idq_uops_not_delivered.cycles_fe_was_ok"           , 0x1, ATTR_INV , 0x0 }, \
 \
{ 0xA0, 0x03, C_ALL, "uop_dispatches_cancelled.simd_prf"                 , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0xA1, 0x01, C_ALL, "uops_executed_port.port_0"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x02, C_ALL, "uops_executed_port.port_1"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x04, C_ALL, "uops_executed_port.port_2"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x08, C_ALL, "uops_executed_port.port_3"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x10, C_ALL, "uops_executed_port.port_4"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x20, C_ALL, "uops_executed_port.port_5"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x40, C_ALL, "uops_executed_port.port_6"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x80, C_ALL, "uops_executed_port.port_7"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x01, C_ALL, "uops_executed_port.port_0_core"                    , 0x0, ATTR_ANY , 0x0 }, \
{ 0xA1, 0x02, C_ALL, "uops_executed_port.port_1_core"                    , 0x0, ATTR_ANY , 0x0 }, \
{ 0xA1, 0x04, C_ALL, "uops_executed_port.port_2_core"                    , 0x0, ATTR_ANY , 0x0 }, \
{ 0xA1, 0x08, C_ALL, "uops_executed_port.port_3_core"                    , 0x0, ATTR_ANY , 0x0 }, \
{ 0xA1, 0x10, C_ALL, "uops_executed_port.port_4_core"                    , 0x0, ATTR_ANY , 0x0 }, \
{ 0xA1, 0x20, C_ALL, "uops_executed_port.port_5_core"                    , 0x0, ATTR_ANY , 0x0 }, \
{ 0xA1, 0x40, C_ALL, "uops_executed_port.port_6_core"                    , 0x0, ATTR_ANY , 0x0 }, \
{ 0xA1, 0x80, C_ALL, "uops_executed_port.port_7_core"                    , 0x0, ATTR_ANY , 0x0 }, \
 \
{ 0xA2, 0x01, C_ALL, "resource_stalls.any"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x04, C_ALL, "resource_stalls.rs"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x08, C_ALL, "resource_stalls.sb"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x10, C_ALL, "resource_stalls.rob"                               , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0xA3, 0x01, C_ALL, "cycle_activity.cycles_l2_pending"                  , 0x1, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x02, C_ALL, "cycle_activity.cycles_ldm_pending"                 , 0x2, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x04, C_ALL, "cycle_activity.cycles_no_execute"                  , 0x4, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x05, C_ALL, "cycle_activity.stalls_l2_pending"                  , 0x5, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x06, C_ALL, "cycle_activity.stalls_ldm_pending"                 , 0x6, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x08, C(2) , "cycle_activity.cycles_l1d_pending"                 , 0x8, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x0C, C(2) , "cycle_activity.stalls_l1d_pending"                 , 0xC, ATTR_NONE, 0x0 }, \
 \
{ 0xA8, 0x01, C_ALL, "lsd.uops"                                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA8, 0x01, C_ALL, "lsd.cycles_active"                                 , 0x1, ATTR_NONE, 0x0 }, \
{ 0xA8, 0x01, C_ALL, "lsd.cycles_4_uops"                                 , 0x4, ATTR_NONE, 0x0 }, \
 \
{ 0xAB, 0x02, C_ALL, "dsb2mite_switches.penalty_cycles"                  , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0xAE, 0x01, C_ALL, "itlb.itlb_flush"                                   , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0xB0, 0x01, C_ALL, "offcore_requests.demand_data_rd"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x02, C_ALL, "offcore_requests.demand_code_rd"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x04, C_ALL, "offcore_requests.demand_rfo"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x08, C_ALL, "offcore_requests.all_data_rd"                      , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0xB1, 0x01, C_ALL, "uops_executed.thread"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x01, C_ALL, "uops_executed.stall_cycles"                        , 0x1, ATTR_INV , 0x0 }, \
{ 0xB1, 0x01, C_ALL, "uops_executed.cycles_ge_1_uop_exec"                , 0x1, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x01, C_ALL, "uops_executed.cycles_ge_2_uops_exec"               , 0x2, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x01, C_ALL, "uops_executed.cycles_ge_3_uops_exec"               , 0x3, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x01, C_ALL, "uops_executed.cycles_ge_4_uops_exec"               , 0x4, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x02, C_ALL, "uops_executed.core"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x02, C_ALL, "uops_executed.core_cycles_none"                    , 0x0, ATTR_INV , 0x0 }, \
{ 0xB1, 0x02, C_ALL, "uops_executed.core_cycles_ge_1"                    , 0x1, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x02, C_ALL, "uops_executed.core_cycles_ge_2"                    , 0x2, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x02, C_ALL, "uops_executed.core_cycles_ge_3"                    , 0x3, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x02, C_ALL, "uops_executed.core_cycles_ge_4"                    , 0x4, ATTR_NONE, 0x0 }, \
 \
{ 0xB2, 0x01, C_ALL, "offcore_requests_buffer.sq_full"                   , 0x0, ATTR_NONE, 0x0 }, \
 \
/* \
 * See Section "Off-core Response Performance Monitoring" \
 * \
 * Though these two off_core events support all counters, only 1 of \
 * them can be used at any given time. This is due to the extra MSR \
 * programming required. \
 */ \
/* { 0xB7, 0x01, C_ALL, "offcore_response_0"                                , 0x0, ATTR_NONE, OFFCORE_RSP_0 }, omit events requiring MSR programming */ \
/* { 0xBB, 0x01, C_ALL, "offcore_response_1"                                , 0x0, ATTR_NONE, OFFCORE_RSP_1 }, omit events requiring MSR programming */ \
 \
{ 0xBC, 0x11, C_ALL, "page_walker_loads.dtlb_l1"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xBC, 0x21, C_ALL, "page_walker_loads.itlb_l1"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xBC, 0x12, C_ALL, "page_walker_loads.dtlb_l2"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xBC, 0x22, C_ALL, "page_walker_loads.itlb_l2"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xBC, 0x14, C_ALL, "page_walker_loads.dtlb_l3"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xBC, 0x24, C_ALL, "page_walker_loads.itlb_l3"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xBC, 0x18, C_ALL, "page_walker_loads.dtlb_memory"                     , 0x0, ATTR_NONE, 0x0 }, \
/* itlb_memory is not in the Intel SDM or spreadsheet for Broadwell;  "cputrack -h" does have it though */ \
{ 0xBC, 0x28, C_ALL, "page_walker_loads.itlb_memory"                     , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0xBD, 0x01, C_ALL, "tlb_flush.dtlb_thread"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xBD, 0x20, C_ALL, "tlb_flush.stlb_any"                                , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0xC0, 0x00, C_ALL, "inst_retired.any_p"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC0, 0x02, C_ALL, "inst_retired.x87"                                  , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0xC1, 0x08, C_ALL, "other_assists.avx_to_sse"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC1, 0x10, C_ALL, "other_assists.sse_to_avx"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC1, 0x40, C_ALL, "other_assists.any_wb_assist"                       , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0xC2, 0x01, C_ALL, "uops_retired.all"                                  , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC2, 0x01, C_ALL, "uops_retired.stall_cycles"                         , 0x1, ATTR_INV , 0x0 }, \
{ 0xC2, 0x01, C_ALL, "uops_retired.total_cycles"                         , 0xA, ATTR_INV , 0x0 }, \
{ 0xC2, 0x01, C_ALL, "uops_retired.core_stall_cycles"                    , 0x1, (ATTR_INV | ATTR_ANY), 0x0 }, \
{ 0xC2, 0x02, C_ALL, "uops_retired.retire_slots"                         , 0x0, ATTR_PEBS, 0x0 }, \
 \
{ 0xC3, 0x01, C_ALL, "machine_clears.cycles"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC3, 0x01, C_ALL, "machine_clears.count"                              , 0x1, ATTR_EDGE, 0x0 }, \
{ 0xC3, 0x02, C_ALL, "machine_clears.memory_ordering"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC3, 0x04, C_ALL, "machine_clears.smc"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC3, 0x20, C_ALL, "machine_clears.maskmov"                            , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0xC4, 0x01, C_ALL, "br_inst_retired.conditional"                       , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC4, 0x02, C_ALL, "br_inst_retired.near_call"                         , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC4, 0x08, C_ALL, "br_inst_retired.near_return"                       , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC4, 0x10, C_ALL, "br_inst_retired.not_taken"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x20, C_ALL, "br_inst_retired.near_taken"                        , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC4, 0x40, C_ALL, "br_inst_retired.far_branch"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x02, C_ALL, "br_inst_retired.near_call_r3"                      , 0x0, ATTR_PEBS, 0x0 }, \
 \
{ 0xC5, 0x01, C_ALL, "br_misp_retired.conditional"                       , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC5, 0x20, C_ALL, "br_misp_retired.near_taken"                        , 0x0, ATTR_PEBS, 0x0 }, \
 \
{ 0xC7, 0x01, C_ALL, "fp_arith_inst_retired.scalar_double"               , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC7, 0x02, C_ALL, "fp_arith_inst_retired.scalar_single"               , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC7, 0x03, C_ALL, "fp_arith_inst_retired.scalar"                      , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC7, 0x04, C_ALL, "fp_arith_inst_retired.128b_packed_double"          , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC7, 0x08, C_ALL, "fp_arith_inst_retired.128b_packed_single"          , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC7, 0x10, C_ALL, "fp_arith_inst_retired.256b_packed_double"          , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC7, 0x15, C_ALL, "fp_arith_inst_retired.double"                      , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC7, 0x20, C_ALL, "fp_arith_inst_retired.256b_packed_single"          , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC7, 0x2A, C_ALL, "fp_arith_inst_retired.single"                      , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC7, 0x3C, C_ALL, "fp_arith_inst_retired.packed"                      , 0x0, ATTR_PEBS, 0x0 }, \
 \
{ 0xC8, 0x01, C_ALL, "hle_retired.start"                                 , 0x0, ATTR_TSX , 0x0 }, \
{ 0xC8, 0x02, C_ALL, "hle_retired.commit"                                , 0x0, ATTR_TSX , 0x0 }, \
{ 0xC8, 0x04, C_ALL, "hle_retired.aborted"                               , 0x0, ATTR_PEBS | ATTR_TSX, 0x0 }, \
{ 0xC8, 0x08, C_ALL, "hle_retired.aborted_misc1"                         , 0x0, ATTR_TSX , 0x0 }, \
{ 0xC8, 0x10, C_ALL, "hle_retired.aborted_misc2"                         , 0x0, ATTR_TSX , 0x0 }, \
{ 0xC8, 0x20, C_ALL, "hle_retired.aborted_misc3"                         , 0x0, ATTR_TSX , 0x0 }, \
{ 0xC8, 0x40, C_ALL, "hle_retired.aborted_misc4"                         , 0x0, ATTR_TSX , 0x0 }, \
{ 0xC8, 0x80, C_ALL, "hle_retired.aborted_misc5"                         , 0x0, ATTR_TSX , 0x0 }, \
 \
{ 0xC9, 0x01, C_ALL, "rtm_retired.start"                                 , 0x0, ATTR_TSX , 0x0 }, \
{ 0xC9, 0x02, C_ALL, "rtm_retired.commit"                                , 0x0, ATTR_TSX , 0x0 }, \
{ 0xC9, 0x04, C_ALL, "rtm_retired.aborted"                               , 0x0, ATTR_PEBS | ATTR_TSX, 0x0 }, \
{ 0xC9, 0x08, C_ALL, "rtm_retired.aborted_misc1"                         , 0x0, ATTR_TSX , 0x0 }, \
{ 0xC9, 0x10, C_ALL, "rtm_retired.aborted_misc2"                         , 0x0, ATTR_TSX , 0x0 }, \
{ 0xC9, 0x20, C_ALL, "rtm_retired.aborted_misc3"                         , 0x0, ATTR_TSX , 0x0 }, \
{ 0xC9, 0x40, C_ALL, "rtm_retired.aborted_misc4"                         , 0x0, ATTR_TSX , 0x0 }, \
{ 0xC9, 0x80, C_ALL, "rtm_retired.aborted_misc5"                         , 0x0, ATTR_TSX , 0x0 }, \
 \
{ 0xCA, 0x02, C_ALL, "fp_assist.x87_output"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCA, 0x04, C_ALL, "fp_assist.x87_input"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCA, 0x08, C_ALL, "fp_assist.simd_output"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCA, 0x10, C_ALL, "fp_assist.simd_input"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCA, 0x1E, C_ALL, "fp_assist.any"                                     , 0x1, ATTR_NONE, 0x0 }, \
 \
{ 0xCC, 0x20, C_ALL, "rob_misc_events.lbr_inserts"                       , 0x0, ATTR_NONE, 0x0 }, \
 \
/* See Section "MSR_PEBS_LD_LAT_THRESHOLD" */ \
/* { 0xCD, 0x01, C(3) , "mem_trans_retired.load_latency"                    , 0x0, ATTR_PEBS_ONLY_LD_LAT, PEBS_LD_LAT_THRESHOLD }, omit events requiring MSR programming */ \
 \
/* \
 * Event 0xD0 must be combined with umasks 0x1(loads) or 0x2(stores) \
 */ \
{ 0xD0, 0x11, C_ALL, "mem_uops_retired.stlb_miss_loads"                  , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD0, 0x12, C_ALL, "mem_uops_retired.stlb_miss_stores"                 , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD0, 0x21, C_ALL, "mem_uops_retired.lock_loads"                       , 0x0, ATTR_PEBS, 0x0 }, \
/* Private event, not public by Intel */ \
{ 0xD0, 0x22, C_ALL, "mem_uops_retired.lock_stores"                      , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD0, 0x41, C_ALL, "mem_uops_retired.split_loads"                      , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD0, 0x42, C_ALL, "mem_uops_retired.split_stores"                     , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD0, 0x81, C_ALL, "mem_uops_retired.all_loads"                        , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD0, 0x82, C_ALL, "mem_uops_retired.all_stores"                       , 0x0, ATTR_PEBS, 0x0 }, \
 \
{ 0xD1, 0x01, C_ALL, "mem_load_uops_retired.l1_hit"                      , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD1, 0x02, C_ALL, "mem_load_uops_retired.l2_hit"                      , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD1, 0x04, C_ALL, "mem_load_uops_retired.l3_hit"                      , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD1, 0x08, C_ALL, "mem_load_uops_retired.l1_miss"                     , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD1, 0x10, C_ALL, "mem_load_uops_retired.l2_miss"                     , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD1, 0x20, C_ALL, "mem_load_uops_retired.l3_miss"                     , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD1, 0x40, C_ALL, "mem_load_uops_retired.hit_lfb"                     , 0x0, ATTR_PEBS, 0x0 }, \
 \
{ 0xD2, 0x01, C_ALL, "mem_load_uops_l3_hit_retired.xsnp_miss"            , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD2, 0x02, C_ALL, "mem_load_uops_l3_hit_retired.xsnp_hit"             , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD2, 0x04, C_ALL, "mem_load_uops_l3_hit_retired.xsnp_hitm"            , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD2, 0x08, C_ALL, "mem_load_uops_l3_hit_retired.xsnp_none"            , 0x0, ATTR_PEBS, 0x0 }, \
 \
{ 0xD3, 0x01, C_ALL, "mem_load_uops_l3_miss_retired.local_dram"          , 0x0, ATTR_PEBS, 0x0 }, \
 \
/* The mem_load_l4_miss_retired events are not in "cputrack -h" output nor in the Intel spreadsheet. */ \
/* { 0xD5, 0x01, C_ALL, "mem_load_l4_miss_retired.local_hit"                , 0x0, ATTR_NONE, 0x0 }, */ \
/* { 0xD5, 0x04, C_ALL, "mem_load_l4_miss_retired.local_miss"               , 0x0, ATTR_NONE, 0x0 }, */ \
 \
{ 0xE6, 0x1F, C_ALL, "baclears.any"                                      , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0xF0, 0x01, C_ALL, "l2_trans.demand_data_rd"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x02, C_ALL, "l2_trans.rfo"                                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x04, C_ALL, "l2_trans.code_rd"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x08, C_ALL, "l2_trans.all_pf"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x10, C_ALL, "l2_trans.l1d_wb"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x20, C_ALL, "l2_trans.l2_fill"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x40, C_ALL, "l2_trans.l2_wb"                                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x80, C_ALL, "l2_trans.all_requests"                             , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0xF1, 0x01, C_ALL, "l2_lines_in.i"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF1, 0x02, C_ALL, "l2_lines_in.s"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF1, 0x04, C_ALL, "l2_lines_in.e"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF1, 0x07, C_ALL, "l2_lines_in.all"                                   , 0x0, ATTR_NONE, 0x0 }, \
 \
{ 0xF2, 0x05, C_ALL, "l2_lines_out.demand_clean"                         , 0x0, ATTR_NONE, 0x0 }, \
/* end of #define */


/* Intel Skylake Processor */
/*
 * This table is essentially taken from:
 *   https://grok.cz.oracle.com/source/xref/on12-clone/usr/src/uts/intel/pcbe/skl_pcbe_tbl.c
 * Also:
 *   https://grok.cz.oracle.com/source/xref/on12-clone/usr/src/uts/intel/pcbe/fam6_pcbe.h
 * { 0xc0, 0x00, C_ALL, "inst_retired.any_p" },		\
 * { 0x3c, 0x01, C_ALL, "cpu_clk_unhalted.ref_p" },	\
 * { 0x2e, 0x4f, C_ALL, "longest_lat_cache.reference" },	\
 * { 0x2e, 0x41, C_ALL, "longest_lat_cache.miss" },	\
 * { 0xc4, 0x00, C_ALL, "br_inst_retired.all_branches" },	\
 * { 0xc5, 0x00, C_ALL, "br_misp_retired.all_branches" }
 * And:
 * https://grok.cz.oracle.com/source/xref/on12-clone/usr/src/uts/intel/pcbe/core_pcbe.c
 *  	{ 0x3c, 0x00, C_ALL, "cpu_clk_unhalted.core" },
 *  	{ 0x3c, 0x00, C_ALL, "cpu_clk_unhalted.thread_p" },
 */
#define EVENTS_FAM6_MOD78 \
{ 0x03, 0x02, C_ALL, "ld_blocks.store_forward"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x03, 0x08, C_ALL, "ld_blocks.no_sr"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x07, 0x01, C_ALL, "ld_blocks_partial.address_alias"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x01, C_ALL, "dtlb_load_misses.miss_causes_a_walk"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x02, C_ALL, "dtlb_load_misses.walk_completed_4k"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x04, C_ALL, "dtlb_load_misses.walk_completed_2m_4m"             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x08, C_ALL, "dtlb_load_misses.walk_completed_1g"                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x0E, C_ALL, "dtlb_load_misses.walk_completed"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x10, C_ALL, "dtlb_load_misses.walk_pending"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x08, 0x10, C_ALL, "dtlb_load_misses.walk_active"                      , 0x1, ATTR_NONE, 0x0 }, \
{ 0x08, 0x20, C_ALL, "dtlb_load_misses.stlb_hit"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0D, 0x01, C_ALL, "int_misc.recovery_cycles"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0D, 0x01, C_ALL, "int_misc.recovery_cycles_any"                      , 0x0, ATTR_ANY, 0x0 }, \
{ 0x0D, 0x80, C_ALL, "int_misc.clear_resteer_cycles"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0E, 0x01, C_ALL, "uops_issued.any"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0E, 0x01, C_ALL, "uops_issued.stall_cycles"                          , 0x1, ATTR_INV, 0x0 }, \
{ 0x0E, 0x02, C_ALL, "uops_issued.vector_width_mismatch"                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x0E, 0x20, C_ALL, "uops_issued.slow_lea"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x14, 0x01, C_ALL, "arith.divider_active"                              , 0x1, ATTR_NONE, 0x0 }, \
{ 0x24, 0x21, C_ALL, "l2_rqsts.demand_data_rd_miss"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x22, C_ALL, "l2_rqsts.rfo_miss"                                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x24, C_ALL, "l2_rqsts.code_rd_miss"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x27, C_ALL, "l2_rqsts.all_demand_miss"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x38, C_ALL, "l2_rqsts.pf_miss"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x3F, C_ALL, "l2_rqsts.miss"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x41, C_ALL, "l2_rqsts.demand_data_rd_hit"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x42, C_ALL, "l2_rqsts.rfo_hit"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0x44, C_ALL, "l2_rqsts.code_rd_hit"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xD8, C_ALL, "l2_rqsts.pf_hit"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xE1, C_ALL, "l2_rqsts.all_demand_data_rd"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xE2, C_ALL, "l2_rqsts.all_rfo"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xE4, C_ALL, "l2_rqsts.all_code_rd"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xE7, C_ALL, "l2_rqsts.all_demand_references"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xF8, C_ALL, "l2_rqsts.all_pf"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x24, 0xFF, C_ALL, "l2_rqsts.references"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2e, 0x4f, C_ALL, "longest_lat_cache.reference"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x2e, 0x41, C_ALL, "longest_lat_cache.miss"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3c, 0x00, C_ALL, "cpu_clk_unhalted.thread_p"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3C, 0x00, C_ALL, "cpu_clk_unhalted.thread_p_any"                     , 0x0, ATTR_ANY, 0x0 }, \
{ 0x3C, 0x00, C_ALL, "cpu_clk_unhalted.ring0_trans"                      , 0x1, ATTR_EDGE, 0x0 }, \
{ 0x3C, 0x01, C_ALL, "cpu_clk_unhalted.ref_p"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3C, 0x01, C_ALL, "cpu_clk_thread_unhalted.ref_xclk"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x3C, 0x01, C_ALL, "cpu_clk_thread_unhalted.ref_xclk_any"              , 0x0, ATTR_ANY, 0x0 }, \
{ 0x3C, 0x02, C_ALL, "cpu_clk_thread_unhalted.one_thread_active"         , 0x0, ATTR_NONE, 0x0 }, \
{ 0x48, 0x01, C_ALL, "l1d_pend_miss.pending"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x48, 0x01, C_ALL, "l1d_pend_miss.pending_cycles"                      , 0x1, ATTR_NONE, 0x0 }, \
{ 0x48, 0x01, C_ALL, "l1d_pend_miss.pending_cycles_any"                  , 0x1, ATTR_ANY, 0x0 }, \
{ 0x48, 0x02, C_ALL, "l1d_pend_miss.fb_full"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x01, C_ALL, "dtlb_store_misses.miss_causes_a_walk"              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x02, C_ALL, "dtlb_store_misses.walk_completed_4k"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x04, C_ALL, "dtlb_store_misses.walk_completed_2m_4m"            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x08, C_ALL, "dtlb_store_misses.walk_completed_1g"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x0E, C_ALL, "dtlb_store_misses.walk_completed"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x10, C_ALL, "dtlb_store_misses.walk_pending"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x49, 0x10, C_ALL, "dtlb_store_misses.walk_active"                     , 0x1, ATTR_NONE, 0x0 }, \
{ 0x49, 0x20, C_ALL, "dtlb_store_misses.stlb_hit"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4C, 0x01, C_ALL, "load_hit_pre.sw_pf"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0x4F, 0x10, C_ALL, "ept.walk_pending"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x51, 0x01, C_ALL, "l1d.replacement"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0x54, 0x01, C_ALL, "tx_mem.abort_conflict"                             , 0x0, ATTR_TSX, 0x0 }, \
{ 0x54, 0x02, C_ALL, "tx_mem.abort_capacity"                             , 0x0, ATTR_TSX, 0x0 }, \
{ 0x54, 0x04, C_ALL, "tx_mem.abort_hle_store_to_elided_lock"             , 0x0, ATTR_TSX, 0x0 }, \
{ 0x54, 0x08, C_ALL, "tx_mem.abort_hle_elision_buffer_not_empty"         , 0x0, ATTR_TSX, 0x0 }, \
{ 0x54, 0x10, C_ALL, "tx_mem.abort_hle_elision_buffer_mismatch"          , 0x0, ATTR_TSX, 0x0 }, \
{ 0x54, 0x20, C_ALL, "tx_mem.abort_hle_elision_buffer_unsupported_alignment",   0x0, ATTR_TSX, 0x0 }, \
{ 0x54, 0x40, C_ALL, "tx_mem.hle_elision_buffer_full"                    , 0x0, ATTR_TSX, 0x0 }, \
{ 0x5D, 0x01, C_ALL, "tx_exec.misc1"                                     , 0x0, ATTR_TSX, 0x0 }, \
{ 0x5D, 0x02, C_ALL, "tx_exec.misc2"                                     , 0x0, ATTR_TSX, 0x0 }, \
{ 0x5D, 0x04, C_ALL, "tx_exec.misc3"                                     , 0x0, ATTR_TSX, 0x0 }, \
{ 0x5D, 0x08, C_ALL, "tx_exec.misc4"                                     , 0x0, ATTR_TSX, 0x0 }, \
{ 0x5D, 0x10, C_ALL, "tx_exec.misc5"                                     , 0x0, ATTR_TSX, 0x0 }, \
{ 0x5E, 0x01, C_ALL, "rs_events.empty_cycles"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x5E, 0x01, C_ALL, "rs_events.empty_end"                               , 0x1, (ATTR_INV | ATTR_EDGE), 0x0 }, \
{ 0x60, 0x01, C_ALL, "offcore_requests_outstanding.demand_data_rd"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x01, C_ALL, "offcore_requests_outstanding.cycles_with_demand_data_rd", 0x1, ATTR_NONE, 0x0 }, \
{ 0x60, 0x01, C_ALL, "offcore_requests_outstanding.demand_data_rd_ge_6"  , 0x6, ATTR_NONE, 0x0 }, \
{ 0x60, 0x02, C_ALL, "offcore_requests_outstanding.demand_code_rd"       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x02, C_ALL, "offcore_requests_outstanding.cycles_with_demand_code_rd", 0x1, ATTR_NONE, 0x0 }, \
{ 0x60, 0x04, C_ALL, "offcore_requests_outstanding.demand_rfo"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x04, C_ALL, "offcore_requests_outstanding.cycles_with_demand_rfo",0x1, ATTR_NONE, 0x0 }, \
{ 0x60, 0x08, C_ALL, "offcore_requests_outstanding.all_data_rd"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x08, C_ALL, "offcore_requests_outstanding.cycles_with_data_rd"  , 0x1, ATTR_NONE, 0x0 }, \
{ 0x60, 0x10, C_ALL, "offcore_requests_outstanding.l3_miss_demand_data_rd",0x0, ATTR_NONE, 0x0 }, \
{ 0x60, 0x10, C_ALL, "offcore_requests_outstanding.cycles_with_l3_miss_demand_data_rd", 0x1, ATTR_NONE, 0x0 }, \
{ 0x60, 0x10, C_ALL, "offcore_requests_outstanding.l3_miss_demand_data_rd_ge_6",0x6, ATTR_NONE, 0x0 }, \
{ 0x79, 0x04, C_ALL, "idq.mite_uops"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x04, C_ALL, "idq.mite_cycles"                                   , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x08, C_ALL, "idq.dsb_uops"                                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x08, C_ALL, "idq.dsb_cycles"                                    , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x10, C_ALL, "idq.ms_dsb_cycles"                                 , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x18, C_ALL, "idq.all_dsb_cycles_4_uops"                         , 0x4, ATTR_NONE, 0x0 }, \
{ 0x79, 0x18, C_ALL, "idq.all_dsb_cycles_any_uops"                       , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x20, C_ALL, "idq.ms_mite_uops"                                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x24, C_ALL, "idq.all_mite_cycles_4_uops"                        , 0x4, ATTR_NONE, 0x0 }, \
{ 0x79, 0x24, C_ALL, "idq.all_mite_cycles_any_uops"                      , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x30, C_ALL, "idq.ms_uops"                                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x79, 0x30, C_ALL, "idq.ms_cycles"                                     , 0x1, ATTR_NONE, 0x0 }, \
{ 0x79, 0x30, C_ALL, "idq.ms_switches"                                   , 0x1, ATTR_EDGE, 0x0 }, \
{ 0x80, 0x04, C_ALL, "icache_16b.ifdata_stall"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0x83, 0x01, C_ALL, "icache_64b.iftag_hit"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x83, 0x02, C_ALL, "icache_64b.iftag_miss"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0x83, 0x04, C_ALL, "icache_64b.iftag_stall"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x01, C_ALL, "itlb_misses.miss_causes_a_walk"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x02, C_ALL, "itlb_misses.walk_completed_4k"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x04, C_ALL, "itlb_misses.walk_completed_2m_4m"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x08, C_ALL, "itlb_misses.walk_completed_1g"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x0E, C_ALL, "itlb_misses.walk_completed"                        , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x10, C_ALL, "itlb_misses.walk_pending"                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0x85, 0x10, C_ALL, "itlb_misses.walk_active"                           , 0x1, ATTR_NONE, 0x0 }, \
{ 0x85, 0x20, C_ALL, "itlb_misses.stlb_hit"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0x87, 0x01, C_ALL, "ild_stall.lcp"                                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0x9C, 0x01, C_ALL, "idq_uops_not_delivered.core"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0x9C, 0x01, C_ALL, "idq_uops_not_delivered.cycles_0_uops_deliv.core"   , 0x4, ATTR_NONE, 0x0 }, \
{ 0x9C, 0x01, C_ALL, "idq_uops_not_delivered.cycles_le_1_uop_deliv.core" , 0x3, ATTR_NONE, 0x0 }, \
{ 0x9C, 0x01, C_ALL, "idq_uops_not_delivered.cycles_le_2_uop_deliv.core" , 0x2, ATTR_NONE, 0x0 }, \
{ 0x9C, 0x01, C_ALL, "idq_uops_not_delivered.cycles_le_3_uop_deliv.core" , 0x1, ATTR_NONE, 0x0 }, \
{ 0x9C, 0x01, C_ALL, "idq_uops_not_delivered.cycles_fe_was_ok"           , 0x1, ATTR_INV, 0x0 }, \
{ 0xA1, 0x01, C_ALL, "uops_dispatched_port.port_0"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x02, C_ALL, "uops_dispatched_port.port_1"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x04, C_ALL, "uops_dispatched_port.port_2"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x08, C_ALL, "uops_dispatched_port.port_3"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x10, C_ALL, "uops_dispatched_port.port_4"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x20, C_ALL, "uops_dispatched_port.port_5"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x40, C_ALL, "uops_dispatched_port.port_6"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA1, 0x80, C_ALL, "uops_dispatched_port.port_7"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x01, C_ALL, "resource_stalls.any"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA2, 0x08, C_ALL, "resource_stalls.sb"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x01, C_ALL, "cycle_activity.cycles_l2_miss"                     , 0x1, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x02, C_ALL, "cycle_activity.cycles_l3_miss"                     , 0x2, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x04, C_ALL, "cycle_activity.stalls_total"                       , 0x4, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x05, C_ALL, "cycle_activity.stalls_l2_miss"                     , 0x5, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x06, C_ALL, "cycle_activity.stalls_l3_miss"                     , 0x6, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x08, C_ALL, "cycle_activity.cycles_l1d_miss"                    , 0x8, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x0C, C_ALL, "cycle_activity.stalls_l1d_miss"                    , 0xC, ATTR_NONE, 0x0 }, \
{ 0xA3, 0x10, C_ALL, "cycle_activity.cycles_mem_any"                     , 0x10,ATTR_NONE, 0x0 }, \
{ 0xA3, 0x14, C_ALL, "cycle_activity.stalls_mem_any"                     , 0x14,ATTR_NONE, 0x0 }, \
{ 0xA6, 0x01, C_ALL, "exe_activity.exe_bound_0_ports"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA6, 0x02, C_ALL, "exe_activity.1_ports_util"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA6, 0x04, C_ALL, "exe_activity.2_ports_util"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA6, 0x08, C_ALL, "exe_activity.3_ports_util"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA6, 0x10, C_ALL, "exe_activity.4_ports_util"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA6, 0x40, C_ALL, "exe_activity.bound_on_stores"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA8, 0x01, C_ALL, "lsd.uops"                                          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xA8, 0x01, C_ALL, "lsd.cycles_active"                                 , 0x1, ATTR_NONE, 0x0 }, \
{ 0xA8, 0x01, C_ALL, "lsd.cycles_4_uops"                                 , 0x4, ATTR_NONE, 0x0 }, \
{ 0xAB, 0x02, C_ALL, "dsb2mite_switches.penalty_cycles"                  , 0x0, ATTR_NONE, 0x0 }, \
{ 0xAE, 0x01, C_ALL, "itlb.itlb_flush"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x01, C_ALL, "offcore_requests.demand_data_rd"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x02, C_ALL, "offcore_requests.demand_code_rd"                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x04, C_ALL, "offcore_requests.demand_rfo"                       , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x08, C_ALL, "offcore_requests.all_data_rd"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x10, C_ALL, "offcore_requests.l3_miss_demand_data_rd"           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB0, 0x80, C_ALL, "offcore_requests.all_requests"                     , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x01, C_ALL, "uops_executed.thread"                              , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x01, C_ALL, "uops_executed.cycles_ge_1_uop_exec"                , 0x1, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x01, C_ALL, "uops_executed.cycles_ge_2_uops_exec"               , 0x2, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x01, C_ALL, "uops_executed.cycles_ge_3_uops_exec"               , 0x3, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x01, C_ALL, "uops_executed.cycles_ge_4_uops_exec"               , 0x4, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x01, C_ALL, "uops_executed.stall_cycles"                        , 0x1, ATTR_INV, 0x0 }, \
{ 0xB1, 0x02, C_ALL, "uops_executed.core"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x02, C_ALL, "uops_executed.core_cycles_none"                    , 0x1, ATTR_INV, 0x0 }, \
{ 0xB1, 0x02, C_ALL, "uops_executed.core_cycles_ge_1"                    , 0x1, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x02, C_ALL, "uops_executed.core_cycles_ge_2"                    , 0x2, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x02, C_ALL, "uops_executed.core_cycles_ge_3"                    , 0x3, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x02, C_ALL, "uops_executed.core_cycles_ge_4"                    , 0x4, ATTR_NONE, 0x0 }, \
{ 0xB1, 0x10, C_ALL, "uops_executed.x87"                                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xB2, 0x01, C_ALL, "offcore_requests_buffer.sq_full"                   , 0x0, ATTR_NONE, 0x0 }, \
\
	/* \
	 * See Section "Off-core Response Performance Monitoring" \
	 * \
	 * Though these two off_core events support all counters, only 1 of \
	 * them can be used at any given time. This is due to the extra MSR \
	 * programming required. \
	 */ \
/* { 0xB7, 0x01, C_ALL, "offcore_response_0"                             , 0x0, ATTR_NONE, OFFCORE_RSP_0 }, omit events requiring MSR programming */ \
/* { 0xBB, 0x01, C_ALL, "offcore_response_1"                             , 0x0, ATTR_NONE, OFFCORE_RSP_1 }, omit events requiring MSR programming */ \
{ 0xBD, 0x01, C_ALL, "tlb_flush.dtlb_thread"                             , 0x0, ATTR_NONE, 0x0 }, \
{ 0xBD, 0x20, C_ALL, "tlb_flush.stlb_any"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc0, 0x00, C_ALL, "inst_retired.any_p"                                , 0x0, ATTR_NONE, 0x0 }, \
/* { 0xC0, 0x1, C(1), "inst_retired.prec_dist"                              , 0x0, ATTR_PEBS_ONLY, 0x0 }, omit PEBS-only events */ \
/* { 0xC0, 0x1, (C(0) | C(2) | C(3)), "inst_retired.total_cycles_ps"        , 0x0A, (ATTR_PEBS_ONLY | ATTR_INV), 0x0 }, omit PEBS-only events */ \
{ 0xC1, 0x3F, C_ALL, "other_assists.any"                                 , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC2, 0x01, C_ALL, "uops_retired.stall_cycles"                         , 0x1, ATTR_INV, 0x0 }, \
{ 0xC2, 0x01, C_ALL, "uops_retired.total_cycles"                         , 0x0A, ATTR_INV, 0x0 }, \
{ 0xC2, 0x02, C_ALL, "uops_retired.retire_slots"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC3, 0x01, C_ALL, "machine_clears.count"                              , 0x1, ATTR_EDGE, 0x0 }, \
{ 0xC3, 0x02, C_ALL, "machine_clears.memory_ordering"                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC3, 0x04, C_ALL, "machine_clears.smc"                                , 0x0, ATTR_NONE, 0x0 }, \
{ 0xc4, 0x00, C_ALL, "br_inst_retired.all_branches"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x01, C_ALL, "br_inst_retired.conditional"                       , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC4, 0x02, C_ALL, "br_inst_retired.near_call"                         , 0x0, ATTR_PEBS, 0x0 }, \
/* { 0xC4, 0x04, C_ALL, "br_inst_retired.all_branches_pebs"                 , 0x0, ATTR_PEBS_ONLY, 0x0 }, omit PEBS-only events */ \
{ 0xC4, 0x08, C_ALL, "br_inst_retired.near_return"                       , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC4, 0x10, C_ALL, "br_inst_retired.not_taken"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC4, 0x20, C_ALL, "br_inst_retired.near_taken"                        , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC4, 0x40, C_ALL, "br_inst_retired.far_branch"                        , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xc5, 0x00, C_ALL, "br_misp_retired.all_branches"                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC5, 0x01, C_ALL, "br_misp_retired.conditional"                       , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xC5, 0x02, C_ALL, "br_misp_retired.near_call"                         , 0x0, ATTR_PEBS, 0x0 }, \
/* { 0xC5, 0x04, C_ALL, "br_misp_retired.all_branches_pebs"                 , 0x0, ATTR_PEBS_ONLY, 0x0 }, omit PEBS-only events */ \
{ 0xC5, 0x20, C_ALL, "br_misp_retired.near_taken"                        , 0x0, ATTR_PEBS, 0x0 }, \
/* { 0xC6, 0x01, C_ALL, "frontend_retired"                                  , 0x0, ATTR_PEBS, MSR_PEBS_FRONTEND}, omit events requiring MSR programming */ \
{ 0xC7, 0x01, C_ALL, "fp_arith_inst_retired.scalar_double"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC7, 0x02, C_ALL, "fp_arith_inst_retired.scalar_single"               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC7, 0x04, C_ALL, "fp_arith_inst_retired.128b_packed_double"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC7, 0x08, C_ALL, "fp_arith_inst_retired.128b_packed_single"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC7, 0x10, C_ALL, "fp_arith_inst_retired.256b_packed_double"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC7, 0x20, C_ALL, "fp_arith_inst_retired.256b_packed_single"          , 0x0, ATTR_NONE, 0x0 }, \
{ 0xC8, 0x01, C_ALL, "hle_retired.start"                                 , 0x0, ATTR_TSX, 0x0 }, \
{ 0xC8, 0x02, C_ALL, "hle_retired.commit"                                , 0x0, ATTR_TSX, 0x0 }, \
{ 0xC8, 0x04, C_ALL, "hle_retired.aborted"                               , 0x0, ATTR_PEBS | ATTR_TSX, 0x0 }, \
{ 0xC8, 0x08, C_ALL, "hle_retired.aborted_mem"                           , 0x0, ATTR_TSX, 0x0 }, \
{ 0xC8, 0x10, C_ALL, "hle_retired.aborted_timer"                         , 0x0, ATTR_TSX, 0x0 }, \
{ 0xC8, 0x20, C_ALL, "hle_retired.aborted_unfriendly"                    , 0x0, ATTR_TSX, 0x0 }, \
{ 0xC8, 0x40, C_ALL, "hle_retired.aborted_memtype"                       , 0x0, ATTR_TSX, 0x0 }, \
{ 0xC8, 0x80, C_ALL, "hle_retired.aborted_events"                        , 0x0, ATTR_TSX, 0x0 }, \
{ 0xC9, 0x01, C_ALL, "rtm_retired.start"                                 , 0x0, ATTR_TSX, 0x0 }, \
{ 0xC9, 0x02, C_ALL, "rtm_retired.commit"                                , 0x0, ATTR_TSX, 0x0 }, \
{ 0xC9, 0x04, C_ALL, "rtm_retired.aborted"                               , 0x0, ATTR_PEBS | ATTR_TSX, 0x0 }, \
{ 0xC9, 0x08, C_ALL, "rtm_retired.aborted_mem"                           , 0x0, ATTR_TSX, 0x0 }, \
{ 0xC9, 0x10, C_ALL, "rtm_retired.aborted_timer"                         , 0x0, ATTR_TSX, 0x0 }, \
{ 0xC9, 0x20, C_ALL, "rtm_retired.aborted_unfriendly"                    , 0x0, ATTR_TSX, 0x0 }, \
{ 0xC9, 0x40, C_ALL, "rtm_retired.aborted_memtype"                       , 0x0, ATTR_TSX, 0x0 }, \
{ 0xC9, 0x80, C_ALL, "rtm_retired.aborted_events"                        , 0x0, ATTR_TSX, 0x0 }, \
{ 0xCA, 0x1E, C_ALL, "fp_assist.any"                                     , 0x1, ATTR_NONE, 0x0 }, \
{ 0xCB, 0x01, C_ALL, "hw_interrupts.received"                            , 0x0, ATTR_NONE, 0x0 }, \
{ 0xCC, 0x20, C_ALL, "rob_misc_events.lbr_inserts"                       , 0x0, ATTR_NONE, 0x0 }, \
/* { 0xCD, 0x01, C_ALL, "mem_trans_retired.load_latency"                    , 0x0, ATTR_PEBS_ONLY_LD_LAT, PEBS_LD_LAT_THRESHOLD }, omit events requiring MSR programming */ \
{ 0xD0, 0x11, C_ALL, "mem_inst_retired.stlb_miss_loads"                 , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD0, 0x12, C_ALL, "mem_inst_retired.stlb_miss_stores"                 , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD0, 0x21, C_ALL, "mem_inst_retired.lock_loads"                       , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD0, 0x41, C_ALL, "mem_inst_retired.split_loads"                      , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD0, 0x42, C_ALL, "mem_inst_retired.split_stores"                     , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD0, 0x81, C_ALL, "mem_inst_retired.all_loads"                        , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD0, 0x82, C_ALL, "mem_inst_retired.all_stores"                       , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD1, 0x01, C_ALL, "mem_load_retired.l1_hit"                           , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD1, 0x02, C_ALL, "mem_load_retired.l2_hit"                           , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD1, 0x04, C_ALL, "mem_load_retired.l3_hit"                           , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD1, 0x08, C_ALL, "mem_load_retired.l1_miss"                          , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD1, 0x10, C_ALL, "mem_load_retired.l2_miss"                          , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD1, 0x20, C_ALL, "mem_load_retired.l3_miss"                          , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD1, 0x40, C_ALL, "mem_load_retired.fb_hit"                           , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD2, 0x01, C_ALL, "mem_load_l3_hit_retired.xsnp_miss"                 , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD2, 0x02, C_ALL, "mem_load_l3_hit_retired.xsnp_hit"                  , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD2, 0x04, C_ALL, "mem_load_l3_hit_retired.xsnp_hitm"                 , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD2, 0x08, C_ALL, "mem_load_l3_hit_retired.xsnp_none"                 , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xD4, 0x04, C_ALL, "mem_load_misc_retired.uc"                          , 0x0, ATTR_PEBS, 0x0 }, \
{ 0xE6, 0x01, C_ALL, "baclears.any"                                      , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF0, 0x40, C_ALL, "l2_trans.l2_wb"                                    , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF1, 0x1F, C_ALL, "l2_lines_in.all"                                   , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x01, C_ALL, "l2_lines_out.silent"                               , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x02, C_ALL, "l2_lines_out.non_silent"                           , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF2, 0x04, C_ALL, "l2_lines_out.useless_hwpf"                         , 0x0, ATTR_NONE, 0x0 }, \
{ 0xF4, 0x10, C_ALL, "sq_misc.split_lock"                                , 0x0, ATTR_NONE, 0x0 }, \
/* end of #define */

#define NT_END {0, 0, 0, NULL, 0x0, ATTR_NONE, 0x0 } /* end-of-table */

static const struct events_table_t *events_table = NULL;

const struct events_table_t events_fam6_mod23[] = {
  ARCH_EVENTS
  EVENTS_FAM6_MOD23
  NT_END
};

const struct events_table_t events_fam6_mod28[] = {
  ARCH_EVENTS
  EVENTS_FAM6_MOD28
  NT_END
};

const struct events_table_t events_fam6_mod26[] = {
  ARCH_EVENTS
  EVENTS_FAM6_MOD26
  NT_END
};

const struct events_table_t events_fam6_mod46[] = {
  ARCH_EVENTS
  EVENTS_FAM6_MOD26
  EVENTS_FAM6_MOD46_ONLY
  NT_END
};

const struct events_table_t events_fam6_mod37[] = {
  ARCH_EVENTS
  EVENTS_FAM6_MOD37
  EVENTS_FAM6_MOD37_ALSO
  NT_END
};

const struct events_table_t events_fam6_mod47[] = {
  ARCH_EVENTS
  EVENTS_FAM6_MOD37
  NT_END
};

const struct events_table_t events_fam6_mod42[] = {
  ARCH_EVENTS
  EVENTS_FAM6_MOD42
  EVENTS_FAM6_MOD42_ONLY
  NT_END
};

const struct events_table_t events_fam6_mod45[] = {
  ARCH_EVENTS
  EVENTS_FAM6_MOD42
  EVENTS_FAM6_MOD45_ONLY
  NT_END
};

const struct events_table_t events_fam6_mod58[] = {
  ARCH_EVENTS
  EVENTS_FAM6_MOD58
  NT_END
};

const struct events_table_t events_fam6_mod62[] = {
  ARCH_EVENTS
  EVENTS_FAM6_MOD58
  EVENTS_FAM6_MOD62_ONLY
  NT_END
};

const struct events_table_t events_fam6_mod60[] = {
  ARCH_EVENTS
  EVENTS_FAM6_MOD60
  NT_END
};

const struct events_table_t events_fam6_mod61[] = {
  ARCH_EVENTS
  EVENTS_FAM6_MOD61
  NT_END
};

const struct events_table_t events_fam6_mod78[] = {
  ARCH_EVENTS
  EVENTS_FAM6_MOD78
  NT_END
};

const struct events_table_t events_fam6_unknown[] = {
  ARCH_EVENTS
  NT_END
};

const struct events_table_t events_fam_arm[] = {
//	ARCH_EVENTS
//    *eventnum = pevent->eventselect;
//    *eventnum |= (pevent->unitmask << PERFCTR_UMASK_SHIFT);
//    *eventnum |= (pevent->attrs << 16);
//    *eventnum |= (pevent->cmask << 24);
// eventselect, unitmask, supported_counters, name, cmask, attrs, msr_offset

// Hardware event
#define HWE(nm, id)     { id, 0, C_ALL, nm, PERF_TYPE_HARDWARE, 0, 0 },
  HWE("branch-instructions",    PERF_COUNT_HW_BRANCH_INSTRUCTIONS)
  HWE("branch-misses",          PERF_COUNT_HW_BRANCH_MISSES)
  HWE("bus-cycles",             PERF_COUNT_HW_BUS_CYCLES)
  HWE("cache-misses",           PERF_COUNT_HW_CACHE_MISSES)
  HWE("cache-references",       PERF_COUNT_HW_CACHE_REFERENCES)
  HWE("cycles",                 PERF_COUNT_HW_CPU_CYCLES)
  HWE("instructions",           PERF_COUNT_HW_INSTRUCTIONS)
  HWE("ref-cycles",             PERF_COUNT_HW_REF_CPU_CYCLES)
  HWE("stalled-cycles-backend", PERF_COUNT_HW_STALLED_CYCLES_BACKEND)
  HWE("stalled-cycles-frontend", PERF_COUNT_HW_STALLED_CYCLES_FRONTEND)

// Software event
#define SWE(nm, id)     { id, 0, C_ALL, nm, PERF_TYPE_SOFTWARE, 0, 0 },
  SWE("alignment-faults",       PERF_COUNT_SW_ALIGNMENT_FAULTS)
  SWE("context-switches",       PERF_COUNT_SW_CONTEXT_SWITCHES)
  SWE("cpu-clock",              PERF_COUNT_SW_CPU_CLOCK)
  SWE("cpu-migrations",         PERF_COUNT_SW_CPU_MIGRATIONS)
  SWE("emulation-faults",       PERF_COUNT_SW_EMULATION_FAULTS)
  SWE("major-faults",           PERF_COUNT_SW_PAGE_FAULTS_MAJ)
  SWE("minor-faults",           PERF_COUNT_SW_PAGE_FAULTS_MIN)
  SWE("page-faults",            PERF_COUNT_SW_PAGE_FAULTS)
  SWE("task-clock",             PERF_COUNT_SW_TASK_CLOCK)

// Hardware cache event
#define HWCE(nm, id, op, res)   { id | (op << 8) | (res << 16), 0, C_ALL, nm, PERF_TYPE_HW_CACHE, 0, 0 },
  HWCE("L1-dcache-load-misses", PERF_COUNT_HW_CACHE_L1D,  PERF_COUNT_HW_CACHE_OP_READ,  PERF_COUNT_HW_CACHE_RESULT_MISS)
  HWCE("L1-dcache-loads",       PERF_COUNT_HW_CACHE_L1D,  PERF_COUNT_HW_CACHE_OP_READ,  PERF_COUNT_HW_CACHE_RESULT_ACCESS)
  HWCE("L1-dcache-store-misses",PERF_COUNT_HW_CACHE_L1D,  PERF_COUNT_HW_CACHE_RESULT_MISS, PERF_COUNT_HW_CACHE_RESULT_ACCESS)
  HWCE("L1-dcache-stores",      PERF_COUNT_HW_CACHE_L1D,  PERF_COUNT_HW_CACHE_OP_WRITE, PERF_COUNT_HW_CACHE_RESULT_ACCESS)
  HWCE("L1-icache-load-misses", PERF_COUNT_HW_CACHE_L1I,  PERF_COUNT_HW_CACHE_OP_READ,  PERF_COUNT_HW_CACHE_RESULT_MISS)
  HWCE("L1-icache-loads",       PERF_COUNT_HW_CACHE_L1I,  PERF_COUNT_HW_CACHE_OP_READ,  PERF_COUNT_HW_CACHE_RESULT_ACCESS)
//    HWCE("branch-load-misses",)
//    HWCE("branch-loads",)
  HWCE("dTLB-load-misses",      PERF_COUNT_HW_CACHE_DTLB, PERF_COUNT_HW_CACHE_OP_READ,  PERF_COUNT_HW_CACHE_RESULT_MISS)
  HWCE("dTLB-loads",            PERF_COUNT_HW_CACHE_DTLB, PERF_COUNT_HW_CACHE_OP_READ,  PERF_COUNT_HW_CACHE_RESULT_ACCESS)
  HWCE("iTLB-load-misses",      PERF_COUNT_HW_CACHE_ITLB, PERF_COUNT_HW_CACHE_OP_READ,  PERF_COUNT_HW_CACHE_RESULT_MISS)
  HWCE("iTLB-loads",            PERF_COUNT_HW_CACHE_ITLB, PERF_COUNT_HW_CACHE_OP_READ,  PERF_COUNT_HW_CACHE_RESULT_ACCESS)

  NT_END
};

static int
core_pcbe_init (void)
{
  switch (cpuid_getvendor ())
    {
    case ARM_CPU_IMP_ARM:
    case ARM_CPU_IMP_BRCM:
    case ARM_CPU_IMP_CAVIUM:
    case ARM_CPU_IMP_APM:
    case ARM_CPU_IMP_QCOM:
      snprintf (core_impl_name, sizeof (core_impl_name), "%s", AARCH64_VENDORSTR_ARM);
      events_table = events_fam_arm;
      num_gpc = 4;  // MEZ: a real implementation is needed
      num_ffc = 0;
      total_pmc = num_gpc + num_ffc;
      return 0;
    case X86_VENDOR_Intel:
      break;
    default:
      return -1;
    }

#if defined(__i386__) || defined(__x86_64)
  /* No Architectural Performance Monitoring Leaf returned by CPUID */
  if (get_cpuid_info ()->cpi_maxeax < 0xa)
    return (-1);

  /* Obtain the Architectural Performance Monitoring Leaf */
  cpuid_regs_t cp;
  my_cpuid (0xa, &cp);
  uint32_t versionid = cp.eax & 0xFF;

  /*
   * Fixed-Function Counters (FFC)
   *
   * All Family 6 Model 15 and Model 23 processors have fixed-function
   * counters.  These counters were made Architectural with
   * Family 6 Model 15 Stepping 9.
   */
  switch (versionid)
    {
    case 0:
      return -1;
    case 2:
      num_ffc = cp.edx & 0x1F;
      /*
       * Some processors have an errata (AW34) where
       * versionid is reported as 2 when actually 1.
       * In this case, fixed-function counters are
       * model-specific as in Version 1.
       */
      if (num_ffc != 0)
	break;
      /* FALLTHROUGH */
    case 1:
      num_ffc = 3;
      versionid = 1;
      break;
    default:
      num_ffc = cp.edx & 0x1F;
      break;
    }
  if (num_ffc >= 64)
    return (-1);
  uint64_t known_ffc_num = sizeof (ffc_names) / sizeof (char *) - 1; /* -1 for EOT */
  if (num_ffc > known_ffc_num)
    /*
     * The system seems to have more fixed-function counters than
     * what this PCBE is able to handle correctly.  Default to the
     * maximum number of fixed-function counters that this driver
     * is aware of.
     */
    num_ffc = known_ffc_num;

  /*
   * General Purpose Counters (GPC)
   */
  num_gpc = (cp.eax >> 8) & 0xFF;
  if (num_gpc >= 64)
    return (-1);
  total_pmc = num_gpc + num_ffc;
  if (total_pmc > 64)   /* Too wide for the overflow bitmap */
    return (-1);

  uint_t cpuid_model = cpuid_getmodel ();

  /* GPC events for Family 6 Models 15 & 23 only */
  if ((cpuid_getfamily () == 6) &&
      ((cpuid_model == 15) || (cpuid_model == 23)))
    (void) snprintf (core_impl_name, IMPL_NAME_LEN, "Core Microarchitecture");
  else
    (void) snprintf (core_impl_name, IMPL_NAME_LEN,
		     "Intel Arch PerfMon v%d on Family %d Model %d",
		     versionid, cpuid_getfamily (), cpuid_model);
  /*
   * Process architectural and non-architectural events using GPC
   */
  if (num_gpc > 0)
    {
      switch (cpuid_model)
	{
	case 15: /* Core 2 */
	case 23:
	  events_table = events_fam6_mod23;
	  break;
	case 28: /* Atom */
	  events_table = events_fam6_mod28;
	  break;
	case 37: /* Westmere */
	case 44:
	  events_table = events_fam6_mod37;
	  break;
	case 47:
	  events_table = events_fam6_mod47;
	  break;
	case 26: /* Nehalem */
	case 30:
	case 31:
	  events_table = events_fam6_mod26;
	  break;
	case 46:
	  events_table = events_fam6_mod46;
	  break;
	case 42: /* Sandy Bridge */
	  events_table = events_fam6_mod42;
	  break;
	case 45:
	  events_table = events_fam6_mod45;
	  break;
	case 58: /* Ivy Bridge */
	  events_table = events_fam6_mod58;
	  break;
	case 62:
	  events_table = events_fam6_mod62;
	  break;
	case 60: /* Haswell */
	case 63:
	case 69:
	case 70:
	  events_table = events_fam6_mod60;
	  break;
	case 61: /* Broadwell */
	case 71:
	case 79:
	case 86:
	  events_table = events_fam6_mod61;
	  break;
	case 78: /* Skylake */
	case 85:
	case 94:
	  events_table = events_fam6_mod78;
	  break;
	default: /* unknown */
	  events_table = events_fam6_unknown;
	}
    }
  /*
   * Fixed-function Counters (FFC) are already listed individually in
   * ffc_names[]
   */
#endif
  return 0;
}

static uint_t
core_pcbe_ncounters ()
{
  return total_pmc;
}

static const char *
core_pcbe_impl_name (void)
{
  return core_impl_name;
}

static const char *
core_pcbe_cpuref (void)
{
#if defined(__aarch64__)
  return "";
#elif defined(__i386__) || defined(__x86_64)
  switch (cpuid_getmodel ())
    {
    case 60: /* Haswell */
    case 63:
    case 69:
    case 70:
      return GTXT ("See Chapter 19 of the \"Intel 64 and IA-32 Architectures Software Developer's Manual Volume 3B: System Programming Guide, Part 2\"\nOrder Number: 253669-047US, June 2013");
    case 61: /* Broadwell */
    case 71:
    case 79:
    case 86:
    case 78: /* Skylake */
    case 85:
    case 94:
      return GTXT ("See Chapter 19 of the \"Intel 64 and IA-32 Architectures Software Developer's Manual Volume 3B: System Programming Guide\"");
    default:
      return
      GTXT ("See Chapter 19 of the \"Intel 64 and IA-32 Architectures Software Developer's Manual Volume 3B: System Programming Guide, Part 2\"\nOrder Number: 253669-045US, January 2013");
    }
#else
  return GTXT ("Unknown cpu model");
#endif
}

static int
core_pcbe_get_events (hwcf_hwc_cb_t *hwc_cb)
{
  int count = 0;
  const struct events_table_t *pevent;
  for (pevent = events_table; pevent && pevent->name; pevent++)
    for (uint_t jj = 0; jj < num_gpc; jj++)
      if (C (jj) & pevent->supported_counters)
	{
	  hwc_cb (jj, pevent->name);
	  count++;
	}

  for (int ii = 0; ii < sizeof (ffc_names) / sizeof (*ffc_names) && ffc_names[ii]; ii++)
    {
      hwc_cb (ii + num_gpc, ffc_names[ii]);
      count++;
    }
  /* add generic events here */
  return count;
}

static int
core_pcbe_get_eventnum (const char *eventname, uint_t pmc, eventsel_t *eventnum,
			eventsel_t *valid_umask, uint_t *pmc_sel)
{
  const struct events_table_t* pevent;
  *valid_umask = 0x0; /* by default, don't allow user umask */
  *pmc_sel = pmc; /* by default, use the requested pmc */

  /* search non-ffc table */
  for (pevent = events_table; pevent && pevent->name; pevent++)
    {
      if (strcmp (eventname, pevent->name) == 0)
	{
	  *eventnum = pevent->eventselect;
	  *eventnum |= (pevent->unitmask << PERFCTR_UMASK_SHIFT);
	  *eventnum |= (pevent->attrs << 16);
	  *eventnum |= (pevent->cmask << 24);

	  if (pevent->msr_offset)
	    {
	      /*
	       * Should also handle any pevent->msr_offset.
	       * Can check libcpc's usr/src/uts/intel/pcbe/snb_pcbe.h,
	       * function snb_gpc_configure().
	       *
	       * Actually, we should probably error out here
	       * until the appropriate support has been added.
	       * Also, we can comment out events that require
	       * msr_offset so that they aren't even listed.
	       */
	    }
	  if (!pevent->unitmask)
	    *valid_umask = 0xff; /* allow umask if nothing set */
	  return 0;
	}
    }

  /* search ffc table */
  for (int ii = 0; ii < sizeof (ffc_names) / sizeof (*ffc_names) && ffc_names[ii]; ii++)
    {
      if (strcmp (eventname, ffc_names[ii]) == 0)
	{
	  *eventnum = 0;
	  *pmc_sel = ii | PERFCTR_FIXED_MAGIC;
	  return 0;
	}
    }
  *eventnum = (eventsel_t) - 1;
  return -1;
}

static hdrv_pcbe_api_t hdrv_pcbe_core_api = {
  core_pcbe_init,
  core_pcbe_ncounters,
  core_pcbe_impl_name,
  core_pcbe_cpuref,
  core_pcbe_get_events,
  core_pcbe_get_eventnum
};
