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

#ifndef _HWCENTRY_H
#define _HWCENTRY_H

#ifndef LIBCOLLECTOR_SRC /* not running in libcollector */
#include <stdio.h>  /* FILE */
#endif  /* --- LIBCOLLECTOR_SRC --- */
#include <stdlib.h> /* size_t */
#include "hwc_cpus.h"
#include "gp-time.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /* ABS backtrack types */
  typedef enum
  {
    /* !! Lowest 2 bits are used to indicate load and store, respectively !! */
    /* Example: On SPARC, backtrack.c did this: if (ABS_memop & inst_type) ... */
    ABST_NONE               = 0x0,
    ABST_LOAD               = 0x1,
    ABST_STORE              = 0x2,
    ABST_LDST               = 0x3,
    ABST_COUNT              = 0x4,
    ABST_US_DTLBM           = 0xF,
    ABST_NOPC               = 0x100,
    ABST_CLKDS              = 0x103,     // Obsolete
    ABST_EXACT              = 0x203,
    ABST_LDST_SPARC64       = 0x303,
    ABST_EXACT_PEBS_PLUS1   = 0x403
    /* full description below... */
  } ABST_type;

#define ABST_PLUS_BY_DEFAULT(n) ((n)==ABST_EXACT || (n)==ABST_EXACT_PEBS_PLUS1)
#define ABST_BACKTRACK_ENABLED(n) ((n)!=ABST_NONE && (n)!=ABST_NOPC)
#define ABST_MEMSPACE_ENABLED(n)  ((n)!=ABST_NONE && (n)!=ABST_NOPC && (n)!=ABST_COUNT)

  /* ABS determines the type of backtracking available for a particular metric.
   * Backtracking is enabled with the "+" in "-h +<countername>...".
   *
   * When Backtracking is not possible:
   *
   *  ABST_NONE=0:     Either the user did not specify "+", or backtracking
   *                   is not applicable to the metric, for example:
   *                     clk cycles,
   *                     instruct counts (dispatch + branch + prefetch),
   *                     i$,
   *                     FP ops
   *  ABST_NOPC=0x100  Used for non-program-related external events, for example:
   *                     system interface events,
   *                     memory controller counters
   *                   Of all ABST_type options, only ABST_NOPC prevents hwprofile.c
   *                     from recording PC/stack information.
   *
   * When backtracking is allowed:
   *
   *  ABST_LOAD=1:     data read events, used with metrics like:
   *                     D$, E$, P$ read misses and hits.
   *                     [DC+EC+PC]_rd*, Re_*_miss*,
   *                     EC_snoop_cb(?)
   *  ABST_STORE=2:    data write events, used with metrics like:
   *                     D$ writes and write related misses
   *                     DC_wr/wr-miss, EC_wb, WC=writecache, Rstall_storeQ
   *                     [EC+PC=pcache]_snoop_inv(?), WC_snoop_cb(?),
   *  ABST_LDST=3:     data reads/writes, used with metrics like:
   *                     E$ references, misses.
   *  ABST_COUNT=4:    dedicated assembly instruction: '%hi(0xfc000)'
   *                     See SW_count_n metric on sparc.
   *  ABST_US_DTLBM=0xF: for load-store on Sparc -- seems to be used only
   *                     for "unskidded DTLB_miss" with DTLB_miss metric.
   *                     Checks two adjacent instructions for Data access.
   *  ABST_CLKDS=0x103: data reads/writes, used with Clock-based Dataspace
   *                     profiling.  Ultrasparc T2 and earlier.
   *  ABST_EXACT=0x203: data reads/writes, precise trap with no skid
   *  ABST_LDST_SPARC64=0x303: Fujitsu SPARC64 load/store
   *  ABST_EXACT_PEBS_PLUS1=0x403: data reads/writes, precise sampling with 1 instr. skid
   */

  /* Hwcentry - structure for defining a counter.
   *   Some fields have different usage when returned from
   *   hwc_lookup(), hwc_post_lookup(), or hwc_scan_*().
   *   Each function will describe its return values in more detail.
   */
  typedef struct
  {
    char *name;         /* user HWC specification */
    char *int_name;     /* internal HWC specification */
    regno_t reg_num;    /* register in CPU, aka picnum, or REGNO_ANY */
    char *metric;       /* descriptive name, for well-known counters only */
    volatile int val;   /* default or actual overflow value */
    int timecvt;        /* multiplier to convert metric to time, 0 if N/A */
    ABST_type memop;    /* type of backtracking allowed */
    char *short_desc;   /* optional one-liner description, or NULL */
    int type;           /* Type of perf_event_attr */
    long long config;   /* perf_event_type -specific configuration */
    /* the fields above this line are expected, in order, by the tables in hwctable.c */
    /* ================================================== */
    /* the fields below this line are more flexible */
    int sort_order;     /* "tag" to associate experiment record with HWC def */
    regno_t *reg_list;  /* if not NULL, legal values for <reg_num> field above */
    /* Note: reg_list will be terminated by REGNO_ANY */
    /* Max size of array is MAX_PICS */
    hrtime_t min_time;  /* target minimum time between overflow events.  0 is off.  See HWCTIME_* macros */
    hrtime_t min_time_default; /* if min_time==HWCTIME_AUTO, use this value instead.  0 is off. */
    int ref_val;    /* if min_time==HWCTIME_AUTO, use this time.  0 is off. */
    int lval, hval; /* temporary to allow DBX to build until dbx glue.cc fixed */
  } Hwcentry;

  // Hwcentry.min_time canned values
#define HWCTIME_TBD ((hrtime_t)( -1LL)) /* self-adjusting enabled but nsecs not yet selected */
#define HWCTIME_HI  (   1 * 1000 * 1000LL ) /*   1 msec represented in nsecs */
#define HWCTIME_ON  (  10 * 1000 * 1000LL ) /*  10 msec represented in nsecs */
#define HWCTIME_LO  ( 100 * 1000 * 1000LL ) /* 100 msec represented in nsecs */

#define HWC_VAL_HI(refVal) (((refVal)/10) + 1)
#define HWC_VAL_ON(refVal) (refVal)
#define HWC_VAL_LO(refVal) (((refVal)*10)/100*100 + 1)  // zero's out lower digits, add 1
#define HWC_VAL_CUSTOM(refVal, targetNanoSec) ((double)(refVal)*(targetNanoSec)/HWCTIME_ON)

#define HWCENTRY_USES_SAMPLING(h)   ((h)->memop==ABST_EXACT_PEBS_PLUS1)

  extern int hwc_lookup (int forKernel, hrtime_t min_time_default,
			 const char *uname, Hwcentry *list[], unsigned listsz,
			 char **emsg, char **wmsg);
  /* Parses counter cmdline string.  Returns counter definitions.
   * Input:
   *   <forKernel> lookup using which table: 0-collect or 1-er_kernel
   *   <min_time_default> minimum nseconds between events if Hwcentry.min_time == HWCTIME_TBD.  0 to disable.
   *   <uname> command line HWC definition of format:
   *           <ctr_def>...[{','|(whitespace)}<ctr_n_def>] where
   *           <ctr_def> == [+]<ctr>[/<reg#>][,<interval>]
   *   <list> array of pointers to store counter definitions
   *   <listsz> number of elements in <list>
   * Returns:
   *   Success:
   *     Returns number of valid counters in <list> and <list>'s elements
   *     will be initialized as follows:
   *
   *     <list[]->name>:
   *       Copy of the <uname> with the following modification:
   *         if backtracking is not supported, the + will be removed.
   *     <list[]->int_name>:
   *       For well-known and convenience ctrs, the internal HWC specification,
   *         e.g. BSQ_cache_reference~emask=0x0100.
   *       For raw ctrs, this will be a copy of <name>.
   *     <list[]->reg_num>:
   *       Register number if specified by user or table, REGNO_ANY otherwise.
   *     <list[]->metric>:
   *       For well-known counters, descriptive name, e.g. "D$ Read Misses".
   *       NULL otherwise.
   *     <list[]->val>:
   *       Overflow value selected by user, default value otherwise.
   *     <list[]->timecvt>:
   *       Value from tables.
   *     <list[]->memop>:
   *       If + is selected and backtracking is allowed, value from table.
   *       ABST_NONE or ABST_NOPC otherwise.
   *
   *     It is the responsibility of the caller to free 'name' and 'int_name'.
   *     'metric' is a static string and shouldn't be freed.
   *     'emsg' will point to NULL
   *
   *   Failure:
   *     Frees all allocated elements.
   *     emsg will point to a string with an error message to print
   *     returns -1
   */

  extern char *hwc_validate_ctrs (int forKernel, Hwcentry *list[], unsigned listsz);
  /* Validates that the vector of specified HW counters can be loaded (more-or-less)
   *   Some invalid combinations, especially on Linux will not be detected
   */

  extern int hwc_get_cpc_cpuver ();
  /* Return the cpc_cpuver for this system.  Other possible values:
   *   CPUVER_GENERIC=0,           CPU could not be determined, but HWCs are ok.
   *   CPUVER_UNDEFINED=-1,        HWCs are not available.
   */

  extern char *hwc_get_docref (char *buf, size_t buflen);
  /* Return a CPU HWC document reference, or NULL. */

  // TBR
  extern char *hwc_get_default_cntrs ();
  /* Return a default HW counter string; may be NULL, or zero-length */
  /* NULL means none is defined in the table; or zero-length means string defined could not be loaded */

  extern char *hwc_get_default_cntrs2 (int forKernel, int style);
  /* like hwc_get_default_cntrs() for style==1 */
  /* but allows other styles of formatting as well */
  /* deprecate and eventually remove hwc_get_default_cntrs() */

  extern char *hwc_get_orig_default_cntrs ();
  /* Get the default HW counter string as set in the table */
  /* NULL means none is defined in the table */

  extern void hwc_update_val (Hwcentry *ctr);
  /* Check time-based intervals and update Hwcentry.val as needed */

  extern char *hwc_get_cpuname (char *buf, size_t buflen);
  /* Return the cpc cpu name for this system, or NULL. */

  extern unsigned hwc_get_max_regs ();
  /* Return number of counters registers for this system. */

  extern unsigned hwc_get_max_concurrent (int forKernel);
  /* Return the max number of simultaneous counters for this system. */

  extern char **hwc_get_attrs (int forKernel);
  /* Return:
   *   Array of attributes (strings) supported by this system.
   *   Last element in array is null.
   *   Array and its elements should NOT be freed by the caller.
   */

  extern unsigned hwc_scan_attrs (void (*action)(const char *attr,
						 const char *desc));
  /* Scan the HW counter attributes, and call function for each attribute.
   * Input:
   *   <action>:
   *     If NULL, no action is performed, but count is still returned.
   *     Otherwise called for each type of attributes, or if none exist,
   *       called once with NULL parameter.
   * Return: count of times <action> would have been called w/ non-NULL data.
   */

  extern Hwcentry *hwc_post_lookup (Hwcentry * pret_ctr, char *uname,
				    char * int_name, int cpc_cpuver);
  /* When post-processing a run, look up a Hwcentry for given type of system.
   * Input:
   *   <pret_ctr>: storage for counter definition
   *   <uname>: well-known name, convenience name, or complete HWC defintion.
   *   <int_name>: Hwcentry->int_name or NULL for don't care
   *   <cpc_cpuver>: version of cpu used for experiment.
   * Return:
   *   <pret_ctr>'s elements set as follows:
   *
   *     <pret_ctr->name>:
   *       Copy of <uname> with the following modifications:
   *         1) + and /<regnum> will be stripped off
   *         2) attributes will be sorted and values will shown in hex.
   *     <pret_ctr->int_name>:
   *       For well-known/convenience counters, the internal HWC specification
   *         from the table, e.g. BSQ_cache_reference~emask=0x0100.
   *       Otherwise, a copy of <uname>.
   *     <pret_ctr->reg_num>:
   *       Register number if specified by user or table,
   *       REGNO_ANY othewise.
   *     <pret_ctr->metric>:
   *       For well-known counters, descriptive name, e.g. "D$ Read Misses".
   *       NULL otherwise.
   *     <pret_ctr->timecvt>:
   *       For well-known/convenience/hidden counters, value from table.
   *       0 otherwise.
   *     <pret_ctr->memop>:
   *       For well-known/convenience/hidden counters, value from table.
   *       ABST_NONE otherwise.
   *     <pret_ctr->sort_order>:
   *       Set to 0.
   *
   *     It is the responsibility of the caller to free 'name' and 'int_name'.
   *     'metric' is a static string and shouldn't be freed.
   */

  extern Hwcentry **hwc_get_std_ctrs (int forKernel);
  /* Return:
   *   Array of well-known counters supported by this system.
   *   Last element in array will be NULL.
   *   Array and its elements should NOT be freed by the caller.
   */

  extern unsigned hwc_scan_std_ctrs (void (*action)(const Hwcentry *));
  /* Call <action> for each well-known counter.
   * Input:
   *   <action>:
   *     If NULL, no action is performed, but count is still returned.
   *     Otherwise called for each type of attributes, or if none exist,
   *       called once with NULL parameter.
   * Return:
   *   Count of times <action> would have been called w/ non-NULL data.
   *   If <action> is not NULL, Hwcentry fields will be set as follows:
   *     <ctr->name>:
   *       HWC alias name, e.g. dcrm.
   *     <ctr->int_name>:
   *       The internal HWC specification, e.g. BSQ_cache_reference~emask=0x0100.
   *     <ctr->reg_num>:
   *       Register number if specified by the table, REGNO_ANY otherwise.
   *     <ctr->metric>:
   *       Descriptive name, e.g. "D$ Read Misses".
   *     <ctr->lval>:
   *       Low-resolution overflow value.
   *     <ctr->val>:
   *       Default overflow value.
   *     <ctr->hval>:
   *       High-resolution overflow value.
   *     <ctr->timecvt>:
   *       multiplier to convert metric to time, 0 otherwise.
   *     <ctr->memop>:
   *       ABST_* type for this counter.
   *     <ctr->reg_list>:
   *       Array of legal <reg_num> values.  Terminated by REGNO_ANY.
   *
   *     Note: All fields point to static data, none should be freed.
   */

  extern Hwcentry **hwc_get_raw_ctrs (int forKernel);
  /* Return:
   *   Table of raw (not well-known) counters supported by this system.
   *   Last element in array will be NULL.
   *   Table and its elements should NOT be freed by the caller.
   */

  extern unsigned hwc_scan_raw_ctrs (void (*action)(const Hwcentry *));
  /* Call <action> for each raw counter.
   * Input:
   *   <action>:
   *     If NULL, no action is performed, but count is still returned.
   *     Otherwise called for each type of attributes, or if none exist,
   *       called once with NULL parameter.
   * Return:
   *   Count of times <action> would have been called w/ non-NULL data.
   *   If <action> is not NULL, Hwcentry fields will be set as follows:
   *     <ctr->name>:
   *       HWC raw name without attributes, e.g. BSQ_cache_reference.
   *     <ctr->int_name>:
   *       NULL.
   *     <ctr->metric>:
   *       NULL.
   *     The remainder of the fields are the same as for
   *       hwc_scan_std_ctrs().
   *
   *     Note: All fields point to static data, none should be freed.
   */

  extern void
  hwc_usage (int forKernel, const char *cmd, const char *dataspace_msg);
  /* Print an i18n'd description of "-h" usage, used by collect and er_kernel.
   */

  extern void hwc_usage_f (int forKernel, FILE *f, const char *cmd,
			   const char *dataspace_msg, int show_syntax,
			   int show_short_desc);
  /* Print an i18n'd description of "-h" usage to a FILE.  Used by GUI. */

  extern char *hwc_rate_string (const Hwcentry *pctr, int force_numeric_format);
  /* Returns {"on"|"hi"|"lo"|""|<value>}.  Return value must be freed by caller. */

  extern char *hwc_i18n_metric (const Hwcentry *ctr);
  /* Get a basic lable for a counter, properly i18n'd.
   *   Note: NOT MT SAFE.
   * Examples:
   *   CPU Cycles
   *   DC_rd Events
   * Pseudocode:
   *   if(ctr->metric != NULL) {
   *	    sprintf(metricbuf, PTXT(ctr->metric) );
   *   } else if (ctr->name != NULL) {
   *	    sprintf(metricbuf, GTXT("%s Events"), ctr->name );
   *   } else if (ctr->int_name != NULL) {
   *	    sprintf(metricbuf, GTXT("%s Events"), ctr->int_name );
   *   }
   * Return: pointer to a buffer containing the above description.
   */

  extern char *hwc_hwcentry_string (char *buf, size_t buflen, const Hwcentry *ctr);
  /* Get a i18n'd description of a HW counter's options.
   *   Examples of well-known counters:
   *     cycles[/{0|1}],9999991 ('CPU Cycles', alias for Cycle_cnt; CPU-cycles)
   *     dcr[/0],1000003 ('D$ Read Refs', alias for DC_rd; load events)
   *   Examples of raw counters:
   *     Cycle_cnt[/{0|1}],1000003 (CPU-cycles)
   *     DC_rd[/0],1000003 (load events)
   * Return: <buf>, filled in.
   */

  extern char *hwc_hwcentry_specd_string (char *buf, size_t buflen, const Hwcentry *ctr);
  /* Get a i18n'd description of a HW counter's specific configuration.
   *   Examples of well-known counters:
   *     cycles,9999991 ('CPU Cycles')
   *     +dcr/0,1000003 ('D$ Read Refs')
   *   Examples of raw counters:
   *     Cycle_cnt,1000003
   *     +DC_rd/0,1000003
   * Return: <buf>, filled in.
   */

  extern const char *hwc_memop_string (ABST_type memop);
  /* Get a i18n'd description of a variable of type ABST_type.
   * Return: pointer to static string.
   */

#ifdef __cplusplus
}
#endif

#endif
