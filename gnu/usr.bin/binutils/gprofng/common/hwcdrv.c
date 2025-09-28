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

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <linux/perf_event.h>

#include "hwcdrv.h"

/*---------------------------------------------------------------------------*/
/* macros */
#define IS_GLOBAL /* Mark global symbols */

#include "cpuid.c" /* ftns for identifying a chip */

static hdrv_pcbe_api_t hdrv_pcbe_core_api;
static hdrv_pcbe_api_t hdrv_pcbe_opteron_api;
static hdrv_pcbe_api_t *hdrv_pcbe_drivers[] = {
  &hdrv_pcbe_core_api,
  &hdrv_pcbe_opteron_api,
  NULL
};
#include "opteron_pcbe.c" /* CPU-specific code */
#include "core_pcbe.c" /* CPU-specific code  */

extern hwcdrv_api_t hwcdrv_pcl_api;
IS_GLOBAL hwcdrv_api_t *hwcdrv_drivers[] = {
  &hwcdrv_pcl_api,
  NULL
};

/*---------------------------------------------------------------------------*/

/* utils for drivers */
IS_GLOBAL int
hwcdrv_assign_all_regnos (Hwcentry* entries[], unsigned numctrs)
{
  unsigned int pmc_assigned[MAX_PICS];
  unsigned idx;
  for (int ii = 0; ii < MAX_PICS; ii++)
    pmc_assigned[ii] = 0;

  /* assign the HWCs that we already know about */
  for (idx = 0; idx < numctrs; idx++)
    {
      regno_t regno = entries[idx]->reg_num;
      if (regno == REGNO_ANY)
	{
	  /* check to see if list of possible registers only contains one entry */
	  regno = REG_LIST_SINGLE_VALID_ENTRY (entries[idx]->reg_list);
	}
      if (regno != REGNO_ANY)
	{
	  if (regno < 0 || regno >= MAX_PICS || !regno_is_valid (entries[idx], regno))
	    {
	      logerr (GTXT ("For counter #%d, register %d is out of range\n"), idx + 1, regno); /*!*/
	      return HWCFUNCS_ERROR_HWCARGS;
	    }
	  TprintfT (DBG_LT2, "hwcfuncs_assign_regnos(): preselected: idx=%d, regno=%d\n", idx, regno);
	  entries[idx]->reg_num = regno; /* assigning back to entries */
	  pmc_assigned[regno] = 1;
	}
    }

  /* assign HWCs that are currently REGNO_ANY */
  for (idx = 0; idx < numctrs; idx++)
    {
      if (entries[idx]->reg_num == REGNO_ANY)
	{
	  int assigned = 0;
	  regno_t *reg_list = entries[idx]->reg_list;
	  for (; reg_list && *reg_list != REGNO_ANY; reg_list++)
	    {
	      regno_t regno = *reg_list;
	      if (regno < 0 || regno >= MAX_PICS)
		{
		  logerr (GTXT ("For counter #%d, register %d is out of range\n"), idx + 1, regno); /*!*/
		  return HWCFUNCS_ERROR_HWCARGS;
		}
	      if (pmc_assigned[regno] == 0)
		{
		  TprintfT (DBG_LT2, "hwcfuncs_assign_regnos(): assigned:   idx=%d, regno=%d\n", idx, regno);
		  entries[idx]->reg_num = regno; /* assigning back to entries */
		  pmc_assigned[regno] = 1;
		  assigned = 1;
		  break;
		}
	    }
	  if (!assigned)
	    {
	      logerr (GTXT ("Counter '%s' could not be bound to a register\n"),
		      entries[idx]->name ? entries[idx]->name : "<NULL>");
	      return HWCFUNCS_ERROR_HWCARGS;
	    }
	}
    }
  return 0;
}

IS_GLOBAL int
hwcdrv_lookup_cpuver (const char * cpcN_cciname)
{
  libcpc2_cpu_lookup_t *plookup;
  static libcpc2_cpu_lookup_t cpu_table[] = {
    LIBCPC2_CPU_LOOKUP_LIST
  };
  if (cpcN_cciname == NULL)
    return CPUVER_UNDEFINED;

  /* search table for name */
  for (plookup = cpu_table; plookup->cpc2_cciname; plookup++)
    {
      int n = strlen (plookup->cpc2_cciname);
      if (!strncmp (plookup->cpc2_cciname, cpcN_cciname, n))
	return plookup->cpc2_cpuver;
    }
  /* unknown, but does have a descriptive string */
  TprintfT (DBG_LT0, "hwcfuncs: CPC2: WARNING: Id of processor '%s' "
	    "could not be determined\n",
	    cpcN_cciname);
  return CPUVER_GENERIC;
}

/*---------------------------------------------------------------------------*/
/* utils to generate x86 register definitions on Linux */

/*
 *  This code is structured as though we're going to initialize the
 *  HWC by writing the Intel MSR register directly.  That is, we
 *  assume the lowest 16 bits of the event number will have the event
 *  and that higher bits will set attributes.
 *
 *  While SPARC is different, we can nonetheless use basically the
 *  same "x86"-named functions:
 *
 *  - The event code will still be 16 bits.  It will still
 *    be in the lowest 16 bits of the event number.  Though
 *    perf_event_code() on SPARC will expect those bits to
 *    shifted, hwcdrv_pcl.c can easily perform that shift.
 *
 *  - On SPARC we support only two attributes, "user" and "system",
 *    which hwcdrv_pcl.c already converts to the "exclude_user"
 *    and "exclude_kernel" fields expected by perf_event_open().
 *    "user" and "system" are stored in event bits 16 and 17.
 *    For M8, a 4-bit mask of supported PICs is stored in bits [23:20].
 */

IS_GLOBAL hwcdrv_get_eventnum_fn_t *hwcdrv_get_x86_eventnum = 0;

static const attr_info_t perfctr_sparc_attrs[] = {
  {NTXT ("user"),   0, 0x01, 16}, //usr
  {NTXT ("system"), 0, 0x01, 17}, //os
  {NULL, 0, 0x00, 0},
};
static const attr_info_t perfctr_x64_attrs[] = {/* ok for Core2 & later */
  {NTXT ("umask"),  0, 0xff, 8},
  {NTXT ("user"),   0, 0x01, 16}, //usr
  //{NTXT("nouser"),  1, 0x01, 16}, //usr (inverted)
  {NTXT ("system"), 0, 0x01, 17}, //os
  {NTXT ("edge"),   0, 0x01, 18},
  {NTXT ("pc"),     0, 0x01, 19},
  {NTXT ("inv"),    0, 0x01, 23},
  {NTXT ("cmask"),  0, 0xff, 24},
  {NULL, 0, 0x00, 0},
};
const attr_info_t *perfctr_attrs_table = perfctr_x64_attrs;

static const eventsel_t perfctr_evntsel_enable_bits = (0x01 << 16) | /* usr */
    // (0xff <<  0) |   /* event*/
    // (0xff <<  8) |   /* umask */
    // (0x01 << 17) |   /* os */
    // (0x01 << 18) |   /* edge */
    // (0x01 << 19) |   /* pc */
    (0x01 << 20) |      /* int */
    // (0x01 << 21) |   /* reserved */
    (0x01 << 22) |      /* enable */
    // (0x01 << 23) |   /* inv */
    // (0xff << 24) |   /* cmask */
    0;

static int
myperfctr_get_x86_eventnum (const char *eventname, uint_t pmc,
			    eventsel_t *eventsel, eventsel_t *valid_umask,
			    uint_t *pmc_sel)
{
  if (hwcdrv_get_x86_eventnum &&
      !hwcdrv_get_x86_eventnum (eventname, pmc, eventsel, valid_umask, pmc_sel))
    return 0;

  /* check for numerically-specified counters */
  char * endptr;
  uint64_t num = strtoull (eventname, &endptr, 0);
  if (*eventname && !*endptr)
    {
      *eventsel = EXTENDED_EVNUM_2_EVSEL (num);
      *valid_umask = 0xff; /* allow any umask (unused for SPARC?) */
      *pmc_sel = pmc;
      return 0;
    }

  /* name does not specify a numeric value */
  *eventsel = (eventsel_t) - 1;
  *valid_umask = 0x0;
  *pmc_sel = pmc;
  return -1;
}

static int
mask_shift_set (eventsel_t *presult, eventsel_t invalue,
		eventsel_t mask, eventsel_t shift)
{
  if (invalue & ~mask)
    return -1; /* invalue attempts to set bits outside of mask */
  *presult &= ~(mask << shift); /* clear all the mask bits */
  *presult |= (invalue << shift); /* set bits according to invalue */
  return 0;
}

static int
set_x86_attr_bits (eventsel_t *result_mask, eventsel_t evnt_valid_umask,
		   hwcfuncs_attr_t attrs[], int nattrs, const char*nameOnly)
{
  eventsel_t evntsel = *result_mask;
  for (int ii = 0; ii < (int) nattrs; ii++)
    {
      const char *attrname = attrs[ii].ca_name;
      eventsel_t attrval = (eventsel_t) attrs[ii].ca_val;
      const char *tmpname;
      int attr_found = 0;
      for (int jj = 0; (tmpname = perfctr_attrs_table[jj].attrname); jj++)
	{
	  if (strcmp (attrname, tmpname) == 0)
	    {
	      if (strcmp (attrname, "umask") == 0)
		{
		  if (attrval & ~evnt_valid_umask)
		    {
		      logerr (GTXT ("for `%s', allowable umask bits are: 0x%llx\n"),
			      nameOnly, (long long) evnt_valid_umask);
		      return -1;
		    }
		}
	      if (mask_shift_set (&evntsel,
				  perfctr_attrs_table[jj].is_inverted ? (attrval^1) : attrval,
				  perfctr_attrs_table[jj].mask,
				  perfctr_attrs_table[jj].shift))
		{
		  logerr (GTXT ("`%s' attribute `%s' could not be set to 0x%llx\n"),
			  nameOnly, attrname, (long long) attrval);
		  return -1;
		}
	      TprintfT (DBG_LT2, "hwcfuncs: Counter %s, attribute %s set to 0x%llx\n",
			nameOnly, attrname, (long long) attrval);
	      attr_found = 1;
	      break;
	    }
	}
      if (!attr_found)
	{
	  logerr (GTXT ("attribute `%s' is invalid\n"), attrname);
	  return -1;
	}
    }
  *result_mask = evntsel;
  return 0;
}

IS_GLOBAL int
hwcfuncs_get_x86_eventsel (unsigned int regno, const char *int_name,
			   eventsel_t *return_event, uint_t *return_pmc_sel)
{
  hwcfuncs_attr_t attrs[HWCFUNCS_MAX_ATTRS + 1];
  unsigned nattrs = 0;
  char *nameOnly = NULL;
  eventsel_t evntsel = 0; // event number
  eventsel_t evnt_valid_umask = 0;
  uint_t pmc_sel = 0;
  int rc = -1;
  *return_event = 0;
  *return_pmc_sel = 0;
  void *attr_mem = hwcfuncs_parse_attrs (int_name, attrs, HWCFUNCS_MAX_ATTRS,
				   &nattrs, NULL);
  if (!attr_mem)
    {
      logerr (GTXT ("out of memory, could not parse attributes\n"));
      return -1;
    }
  hwcfuncs_parse_ctr (int_name, NULL, &nameOnly, NULL, NULL, NULL);
  if (regno == REGNO_ANY)
    {
      logerr (GTXT ("reg# could not be determined for `%s'\n"), nameOnly);
      goto attr_wrapup;
    }

  /* look up evntsel */
  if (myperfctr_get_x86_eventnum (nameOnly, regno,
				  &evntsel, &evnt_valid_umask, &pmc_sel))
    {
      logerr (GTXT ("counter `%s' is not valid\n"), nameOnly);
      goto attr_wrapup;
    }
  TprintfT (DBG_LT1, "hwcfuncs: event=0x%llx pmc=0x%x '%s' nattrs = %u\n",
	    (long long) evntsel, pmc_sel, nameOnly, nattrs);

  /* determine event attributes */
  eventsel_t evnt_attrs = perfctr_evntsel_enable_bits;
  if (set_x86_attr_bits (&evnt_attrs, evnt_valid_umask, attrs, nattrs, nameOnly))
    goto attr_wrapup;
  if (evntsel & evnt_attrs)
    TprintfT (DBG_LT0, "hwcfuncs: ERROR - evntsel & enable bits overlap: 0x%llx 0x%llx 0x%llx\n",
	      (long long) evntsel, (long long) evnt_attrs,
	      (long long) (evntsel & evnt_attrs));
  *return_event = evntsel | evnt_attrs;
  *return_pmc_sel = pmc_sel;
  rc = 0;

attr_wrapup:
  free (attr_mem);
  free (nameOnly);
  return rc;
}

#ifdef __x86_64__
#define syscall_instr          "syscall"
#define syscall_clobber        "rcx", "r11", "memory"
#endif
#ifdef __i386__
#define syscall_instr          "int $0x80"
#define syscall_clobber        "memory"
#endif

static inline int
perf_event_open (struct perf_event_attr *hw_event_uptr, pid_t pid,
		 int cpu, int group_fd, unsigned long flags)
{
  /* It seems that perf_event_open() sometimes fails spuriously,
   * even while an immediate retry succeeds.
   * So, let's try a few retries if the call fails just to be sure.
   */
  int rc;
  for (int retry = 0; retry < 5; retry++)
    {
      rc = syscall (__NR_perf_event_open, hw_event_uptr, pid, cpu, group_fd, flags);
      if (rc != -1)
	return rc;
    }
  return rc;
}

/*---------------------------------------------------------------------------*/
/* macros & fwd prototypes */

#define HWCDRV_API      static /* Mark functions used by hwcdrv API */

HWCDRV_API int hwcdrv_start (void);
HWCDRV_API int hwcdrv_free_counters ();

static pid_t
hwcdrv_gettid (void)
{
#ifndef LIBCOLLECTOR_SRC
  return syscall (__NR_gettid);
#elif defined(intel)
  pid_t r;
  __asm__ __volatile__(syscall_instr
		       : "=a" (r) : "0" (__NR_gettid)
		       : syscall_clobber);
  return r;
#else
  return syscall (__NR_gettid); // FIXUP_XXX_SPARC_LINUX // write gettid in asm
#endif
}

/*---------------------------------------------------------------------------*/
/* types */

#define NPAGES_PER_BUF  1 // number of pages to be used for perf_event samples
// must be a power of 2

/*---------------------------------------------------------------------------*/

/* typedefs */

typedef struct
{ // event (hwc) definition
  unsigned int reg_num; // PMC assignment, potentially for detecting conflicts
  eventsel_t eventsel;          // raw event bits (Intel/AMD)
  uint64_t counter_preload;     // number of HWC events before signal
  struct perf_event_attr hw;    // perf_event definition
  hrtime_t min_time;            // minimum time we're targeting between events
  char *name;
} perf_event_def_t;

typedef struct
{ // runtime state of perf_event buffer
  void *buf;                    // pointer to mmapped buffer
  size_t pagesz;                // size of pages
} buffer_state_t;

typedef struct
{ // runtime state of counter values
  uint64_t prev_ena_ts;         // previous perf_event "enabled" time
  uint64_t prev_run_ts;         // previous perf_event "running" time
  uint64_t prev_value;          // previous HWC value
} counter_value_state_t;

typedef struct
{ // per-counter information
  perf_event_def_t *ev_def;     // global HWC definition for one counter
  int fd;                       // perf_event fd
  buffer_state_t buf_state;     // perf_event buffer's state
  counter_value_state_t value_state; // counter state
  int needs_restart;            // workaround for dbx failure to preserve si_fd
  uint64_t last_overflow_period;
  hrtime_t last_overflow_time;
} counter_state_t;

typedef struct
{ // per-thread context
  counter_state_t *ctr_list;
  int signal_fd;                // fd that caused the most recent signal
  pid_t tid;			// for debugging signal delivery problems
} hdrv_pcl_ctx_t;

/*---------------------------------------------------------------------------*/

/* static variables */
static struct
{
  int library_ok;
  int internal_open_called;
  hwcfuncs_tsd_get_fn_t find_vpc_ctx;
  unsigned hwcdef_cnt;      /* number of *active* hardware counters */
  hwcdrv_get_events_fn_t *get_events;
} hdrv_pcl_state;

static hwcdrv_about_t hdrv_pcl_about = {.cpcN_cpuver = CPUVER_UNDEFINED};
static perf_event_def_t global_perf_event_def[MAX_PICS];

#define COUNTERS_ENABLED()      (hdrv_pcl_state.hwcdef_cnt)


/* perf_event buffer formatting and handling */
static void
reset_buf (buffer_state_t *bufstate)
{
  TprintfT (0, "hwcdrv: ERROR: perf_event reset_buf() called!\n");
  struct perf_event_mmap_page *metadata = bufstate->buf;
  if (metadata)
    metadata->data_tail = metadata->data_head;
}

static int
skip_buf (buffer_state_t *bufstate, size_t sz)
{
  TprintfT (DBG_LT1, "hwcdrv: WARNING: perf_event skip_buf called!\n");
  struct perf_event_mmap_page *metadata = bufstate->buf;
  if (metadata == NULL)
    return -1;
  size_t pgsz = bufstate->pagesz;
  size_t bufsz = NPAGES_PER_BUF*pgsz;
  uint64_t d_tail = metadata->data_tail;
  uint64_t d_head = metadata->data_head;

  // validate request size
  if (sz > d_head - d_tail || sz >= bufsz)
    {
      reset_buf (bufstate);
      return -1;
    }
  metadata->data_tail = d_tail + sz; // advance tail
  return 0;
}

static int
read_buf (buffer_state_t *bufstate, void *buf, size_t sz)
{
  struct perf_event_mmap_page *metadata = bufstate->buf;
  if (metadata == NULL)
    return -1;
  size_t pgsz = bufstate->pagesz;
  size_t bufsz = NPAGES_PER_BUF*pgsz;
  uint64_t d_tail = metadata->data_tail;
  uint64_t d_head = metadata->data_head;

  // validate request size
  if (sz > d_head - d_tail || sz >= bufsz)
    {
      reset_buf (bufstate);
      return -1;
    }
  char *buf_base = ((char *) metadata) + pgsz; // start of data buffer
  uint64_t start_pos = d_tail & (bufsz - 1); // char offset into data buffer
  size_t nbytes = sz;
  if (start_pos + sz > bufsz)
    {
      // will wrap past end of buffer
      nbytes = bufsz - start_pos;
      memcpy (buf, buf_base + start_pos, nbytes);
      start_pos = 0; // wrap to start
      buf = (void *) (((char *) buf) + nbytes);
      nbytes = sz - nbytes;
    }
  memcpy (buf, buf_base + start_pos, nbytes);
  metadata->data_tail += sz;
  return 0;
}

static int
read_u64 (buffer_state_t *bufstate, uint64_t *value)
{
  return read_buf (bufstate, value, sizeof (uint64_t));
}

static int
read_sample (counter_state_t *ctr_state, int msgsz, uint64_t *rvalue,
	     uint64_t *rlost)
{
  // returns count of bytes read
  buffer_state_t *bufstate = &ctr_state->buf_state;
  counter_value_state_t *cntstate = &ctr_state->value_state;
  int readsz = 0;

  // PERF_SAMPLE_IP
  uint64_t ipc = 0;
  int rc = read_u64 (bufstate, &ipc);
  if (rc)
    return -1;
  readsz += sizeof (uint64_t);

  // PERF_SAMPLE_READ: value
  uint64_t value = 0;
  rc = read_u64 (bufstate, &value);
  if (rc)
    return -2;
  readsz += sizeof (uint64_t);

  /* Bug 20806896
   * Old Linux kernels (e.g. 2.6.32) on certain systems return enabled and
   * running times in the sample data that correspond to the metadata times
   *     metadata->time_enabled
   *     metadata->time_running
   * from the PREVIOUS (not current) sample.  Probably just ignore this bug
   * since it's on old kernels and we only use the enabled and running times
   * to construct loss_estimate.
   */
  // PERF_SAMPLE_READ: PERF_FORMAT_ENABLED
  uint64_t enabled_time = 0;
  rc = read_u64 (bufstate, &enabled_time);
  if (rc)
    return -3;
  readsz += sizeof (uint64_t);

  // PERF_SAMPLE_READ: PERF_FORMAT_RUNNING
  uint64_t running_time = 0;
  rc = read_u64 (bufstate, &running_time);
  if (rc)
    return -4;
  readsz += sizeof (uint64_t);

  uint64_t value_delta = value - cntstate->prev_value;
  uint64_t enabled_delta = enabled_time - cntstate->prev_ena_ts;
  uint64_t running_delta = running_time - cntstate->prev_run_ts;
  cntstate->prev_value = value;
  cntstate->prev_ena_ts = enabled_time;
  cntstate->prev_run_ts = running_time;

  // 24830461 need workaround for Linux anomalous HWC skid overrun
  int set_error_flag = 0;
  if (value_delta > 2 * ctr_state->last_overflow_period + 2000 /* HWC_SKID_TOLERANCE */)
    set_error_flag = 1;

  uint64_t loss_estimate = 0; // estimate loss of events caused by multiplexing
  if (running_delta == enabled_delta)
    {
      // counter was running 100% of time, no multiplexing
    }
  else if (running_delta == 0)
    loss_estimate = 1; // token amount to aid in debugging perfctr oddities
  else if ((running_delta > enabled_delta) || (enabled_delta & 0x1000000000000000ll))
    {
      // running should be smaller than enabled, can't estimate
      /*
       * 21418391 HWC can have a negative count
       *
       * We've also seen enabled not only be smaller than running
       * but in fact go negative.  Guard against this.
       */
      loss_estimate = 2; // token amount to aid in debugging perfctr oddities
    }
  else
    {
      // counter was running less than 100% of time
      // Example: ena=7772268 run=6775669 raw_value=316004 scaled_value=362483 loss_est=46479
      uint64_t scaled_delta = (double) value_delta * enabled_delta / running_delta;
      value_delta = scaled_delta;
#if 0
      // We should perhaps warn the user that multiplexing is going on,
      // but hwcdrv_pcl.c doesn't know about the collector_interface, SP_JCMD_COMMENT, or COL_COMMENT_* values.
      // For now we simply don't report.
      // Perhaps we should address the issue not here but in the caller collector_sigemt_handler(),
      // but at that level "lost" has a meaning that's considerably broader than just multiplexing.
      collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">%s %d -> %d</event>\n",
				     SP_JCMD_COMMENT, COL_COMMENT_HWCADJ, global_perf_event_def[idx].name,
				     ctr_list[idx].last_overflow_period, new_period);
#endif
    }
  TprintfT ((loss_estimate || set_error_flag) ? DBG_LT1 : DBG_LT3,
	    "hwcdrv: '%s' ipc=0x%llx ena=%llu run=%llu "
	    "value_delta=%lld(0x%llx) loss_est=%llu %s error_flag='%s'\n",
	    ctr_state->ev_def->name, (long long) ipc,
	    (long long) enabled_delta, (long long) running_delta,
	    (long long) value_delta, (long long) value_delta,
	    (unsigned long long) loss_estimate,
	    loss_estimate ? ", WARNING - SCALED" : "",
	    set_error_flag ? ", ERRORFLAG" : "");
  if (set_error_flag == 1)
    value_delta |= (1ULL << 63)     /* HWCVAL_ERR_FLAG */;
  *rvalue = value_delta;
  *rlost = loss_estimate;
  if (readsz != msgsz)
    {
      TprintfT (0, "hwcdrv: ERROR: perf_event sample not fully parsed\n");
      return -5;
    }
  return 0;
}

static void
dump_perf_event_attr (struct perf_event_attr *at)
{
  TprintfT (DBG_LT2, "dump_perf_event_attr:  size=%d  type=%d  sample_period=%lld\n"
	    "  config=0x%llx  config1=0x%llx  config2=0x%llx  wakeup_events=%lld __reserved_1=%lld\n",
	    (int) at->size, (int) at->type, (unsigned long long) at->sample_period,
	    (unsigned long long) at->config, (unsigned long long) at->config1,
	    (unsigned long long) at->config2, (unsigned long long) at->wakeup_events,
	    (unsigned long long) at->__reserved_1);
#define DUMP_F(fld) if (at->fld) TprintfT(DBG_LT2, "  %-10s : %lld\n", #fld, (long long) at->fld)
  DUMP_F (disabled);
  DUMP_F (inherit);
  DUMP_F (pinned);
  DUMP_F (exclusive);
  DUMP_F (exclude_user);
  DUMP_F (exclude_kernel);
  DUMP_F (exclude_hv);
  DUMP_F (exclude_idle);
  //    DUMP_F(xmmap);
  DUMP_F (comm);
  DUMP_F (freq);
  DUMP_F (inherit_stat);
  DUMP_F (enable_on_exec);
  DUMP_F (task);
  DUMP_F (watermark);
}

static void
init_perf_event (struct perf_event_attr *hw, uint64_t event, uint64_t period)
{
  memset (hw, 0, sizeof (struct perf_event_attr));
  hw->size = sizeof (struct perf_event_attr); // fwd/bwd compat

#if defined(__i386__) || defined(__x86_64)
  //note: Nehalem/Westmere OFFCORE_RESPONSE in upper 32 bits
  hw->config = event;
  hw->type = PERF_TYPE_RAW;     // hw/sw/trace/raw...
#elif defined(__aarch64__)
  hw->type = (event >> 24) & 7;
  hw->config = event & 0xff;
#elif defined(sparc)
  //SPARC needs to be shifted up 16 bits
  hw->config = (event & 0xFFFF) << 16;  // uint64_t event
  uint64_t regs = (event >> 20) & 0xf;  // see sparc_pcbe.c
  hw->config |= regs << 4;  // for M8, supported PICs need to be placed at bits [7:4]
  hw->type = PERF_TYPE_RAW; // hw/sw/trace/raw...
#endif

  hw->sample_period = period;
  hw->sample_type = PERF_SAMPLE_IP |
	  // PERF_SAMPLE_TID		|
	  // PERF_SAMPLE_TIME		| // possibly interesting
	  // PERF_SAMPLE_ADDR		|
	  PERF_SAMPLE_READ | // HWC value
	  // PERF_SAMPLE_CALLCHAIN	| // interesting
	  // PERF_SAMPLE_ID		|
	  // PERF_SAMPLE_CPU		| // possibly interesting
	  // PERF_SAMPLE_PERIOD		|
	  // PERF_SAMPLE_STREAM_ID	|
	  // PERF_SAMPLE_RAW		|
	  0;
  hw->read_format =
	  PERF_FORMAT_TOTAL_TIME_ENABLED | // detect when hwc not scheduled
	  PERF_FORMAT_TOTAL_TIME_RUNNING | // detect when hwc not scheduled
	  // PERF_FORMAT_ID		|
	  // PERF_FORMAT_GROUP		|
	  0;
  hw->disabled = 1; /* off by default */

  // Note: the following override config.priv bits!
  hw->exclude_user = (event & (1 << 16)) == 0;      /* don't count user */
  hw->exclude_kernel = (event & (1 << 17)) == 0;    /* ditto kernel */
  hw->exclude_hv = 1;       /* ditto hypervisor */
  hw->wakeup_events = 1;    /* wakeup every n events */
  dump_perf_event_attr (hw);
}

static int
start_one_ctr (int ii, size_t pgsz, hdrv_pcl_ctx_t * pctx, char *error_string)
{
  // pe_attr should have been initialized in hwcdrv_create_counters()
  struct perf_event_attr pe_attr;
  memcpy (&pe_attr, &global_perf_event_def[ii].hw, sizeof (pe_attr));

  // but we adjust the period, so make sure that pctx->ctr_list[ii].last_overflow_period has been set
  pe_attr.sample_period = pctx->ctr_list[ii].last_overflow_period;

  int hwc_fd = perf_event_open (&pe_attr, pctx->tid, -1, -1, 0);
  if (hwc_fd == -1)
    {
      TprintfT (DBG_LT1, "%s idx=%d perf_event_open failed, errno=%d\n",
		error_string, ii, errno);
      return 1;
    }

  size_t buffer_area_sz = (NPAGES_PER_BUF + 1) * pgsz; // add a page for metadata
  void * buf = mmap (NULL, buffer_area_sz, //YXXX is this a safe call?
		     PROT_READ | PROT_WRITE, MAP_SHARED, hwc_fd, 0);
  if (buf == MAP_FAILED)
    {
      TprintfT (0, "sz = %ld, pgsz = %ld\n  err=%s idx=%d mmap failed: %s\n",
		(long) buffer_area_sz, (long) pgsz, error_string, ii, strerror (errno));
      return 1;
    }
  pctx->ctr_list[ii].ev_def = &global_perf_event_def[ii]; // why do we set ev_def?  we never seem to use it
  pctx->ctr_list[ii].fd = hwc_fd;
  pctx->ctr_list[ii].buf_state.buf = buf;
  pctx->ctr_list[ii].buf_state.pagesz = pgsz;
  pctx->ctr_list[ii].value_state.prev_ena_ts = 0;
  pctx->ctr_list[ii].value_state.prev_run_ts = 0;
  pctx->ctr_list[ii].value_state.prev_value = 0;
  pctx->ctr_list[ii].last_overflow_time = gethrtime ();

  /* set async mode */
  long flags = fcntl (hwc_fd, F_GETFL, 0) | O_ASYNC;
  int rc = fcntl (hwc_fd, F_SETFL, flags);
  if (rc == -1)
    {
      TprintfT (0, "%s idx=%d O_ASYNC failed\n", error_string, ii);
      return 1;
    }

  /*
   * set lwp ownership of the fd
   * See BUGS section of "man perf_event_open":
   *     The F_SETOWN_EX option to fcntl(2) is needed to properly get
   *     overflow signals in threads.  This was introduced in Linux 2.6.32.
   * Legacy references:
   *     see http://lkml.org/lkml/2009/8/4/128
   *     google man fcntl F_SETOWN_EX -conflict
   *       "From Linux 2.6.32 onward, use F_SETOWN_EX to target
   *       SIGIO and SIGURG signals at a particular thread."
   *     http://icl.cs.utk.edu/papi/docs/da/d2a/examples__v2_8x_2self__smpl__multi_8c.html
   *     See 2010 CSCADS presentation by Eranian
   */
  struct f_owner_ex fowner_ex;
  fowner_ex.type = F_OWNER_TID;
  fowner_ex.pid = pctx->tid;
  rc = fcntl (hwc_fd, F_SETOWN_EX, (unsigned long) &fowner_ex);
  if (rc == -1)
    {
      TprintfT (0, "%s idx=%d F_SETOWN failed\n", error_string, ii);
      return 1;
    }

  /* Use sigio so handler can determine FD via siginfo->si_fd. */
  rc = fcntl (hwc_fd, F_SETSIG, SIGIO);
  if (rc == -1)
    {
      TprintfT (0, "%s idx=%d F_SETSIG failed\n", error_string, ii);
      return 1;
    }
  return 0;
}

static int
stop_one_ctr (int ii, counter_state_t *ctr_list)
{
  int hwc_rc = 0;
  if (-1 == ioctl (ctr_list[ii].fd, PERF_EVENT_IOC_DISABLE, 1))
    {
      TprintfT (0, "hwcdrv: ERROR: PERF_EVENT_IOC_DISABLE #%d failed: errno=%d\n", ii, errno);
      hwc_rc = HWCFUNCS_ERROR_GENERIC;
    }
  void *buf = ctr_list[ii].buf_state.buf;
  if (buf)
    {
      size_t bufsz = (NPAGES_PER_BUF + 1) * ctr_list[ii].buf_state.pagesz;
      ctr_list[ii].buf_state.buf = NULL;
      int tmprc = munmap (buf, bufsz);
      if (tmprc)
	{
	  TprintfT (0, "hwcdrv: ERROR: munmap() #%d failed: errno=%d\n", ii, errno);
	  hwc_rc = HWCFUNCS_ERROR_GENERIC;
	}
    }
  if (-1 == close (ctr_list[ii].fd))
    {
      TprintfT (0, "hwcdrv: ERROR: close(fd) #%d failed: errno=%d\n", ii, errno);
      hwc_rc = HWCFUNCS_ERROR_GENERIC;
    }
  return hwc_rc;
}

/* HWCDRV_API for thread-specific actions */
HWCDRV_API int
hwcdrv_lwp_init (void)
{
  return hwcdrv_start ();
}

HWCDRV_API void
hwcdrv_lwp_fini (void)
{
  hwcdrv_free_counters ();  /* also sets pctx->ctr_list=NULL; */
}

/* open */
static int
hdrv_pcl_internal_open ()
{
  if (hdrv_pcl_state.internal_open_called)
    {
      TprintfT (0, "hwcdrv: WARNING: hdrv_pcl_internal_open: already called\n");
      return HWCFUNCS_ERROR_ALREADY_CALLED;
    }

  // determine if PCL is available
  perf_event_def_t tmp_event_def;
  memset (&tmp_event_def, 0, sizeof (tmp_event_def));
  struct perf_event_attr *pe_attr = &tmp_event_def.hw;
  init_perf_event (pe_attr, 0, 0);
  pe_attr->type = PERF_TYPE_HARDWARE; // specify abstracted HW event
  pe_attr->config = PERF_COUNT_HW_INSTRUCTIONS; // specify abstracted insts
  int hwc_fd = perf_event_open (pe_attr,
				0, // pid/tid, 0 is self
				-1, // cpu, -1 is per-thread mode
				-1, // group_fd, -1 is root
				0); // flags
  if (hwc_fd == -1)
    {
      TprintfT (DBG_LT1, "hwcdrv: WARNING: hdrv_pcl_internal_open:"
		" perf_event_open() failed, errno=%d\n", errno);
      goto internal_open_error;
    }

  /* see if the PCL is new enough to know about F_SETOWN_EX */
  struct f_owner_ex fowner_ex;
  fowner_ex.type = F_OWNER_TID;
  fowner_ex.pid = hwcdrv_gettid (); // "pid=tid" is correct w/F_OWNER_TID
  if (fcntl (hwc_fd, F_SETOWN_EX, (unsigned long) &fowner_ex) == -1)
    {
      TprintfT (DBG_LT1, "hwcdrv: WARNING: hdrv_pcl_internal_open: "
		"F_SETOWN failed, errno=%d\n", errno);
      close (hwc_fd);
      goto internal_open_error;
    }
  close (hwc_fd);

  hdrv_pcl_state.internal_open_called = 1;
  hdrv_pcl_state.library_ok = 1; // set to non-zero to show it's initted
  hdrv_pcl_about.cpcN_cpuver = CPUVER_UNDEFINED;
  TprintfT (DBG_LT2, "hwcdrv: hdrv_pcl_internal_open()\n");
  for (int ii = 0; hdrv_pcbe_drivers[ii]; ii++)
    {
      hdrv_pcbe_api_t *ppcbe = hdrv_pcbe_drivers[ii];
      if (!ppcbe->hdrv_pcbe_init ())
	{
	  hdrv_pcl_about.cpcN_cciname = ppcbe->hdrv_pcbe_impl_name ();
	  hdrv_pcl_about.cpcN_cpuver = hwcdrv_lookup_cpuver (hdrv_pcl_about.cpcN_cciname);
	  if (hdrv_pcl_about.cpcN_cpuver == CPUVER_UNDEFINED)
	    goto internal_open_error;
	  hdrv_pcl_about.cpcN_npics = ppcbe->hdrv_pcbe_ncounters ();
	  hdrv_pcl_about.cpcN_docref = ppcbe->hdrv_pcbe_cpuref ();
	  hdrv_pcl_state.get_events = ppcbe->hdrv_pcbe_get_events;
	  hwcdrv_get_x86_eventnum = ppcbe->hdrv_pcbe_get_eventnum;
	  break;
	}
    }
  if (hdrv_pcl_about.cpcN_npics > MAX_PICS)
    {
      TprintfT (0, "hwcdrv: WARNING: hdrv_pcl_internal_open:"
		" reducing number of HWCs from %u to %u on processor '%s'\n",
		hdrv_pcl_about.cpcN_npics, MAX_PICS, hdrv_pcl_about.cpcN_cciname);
      hdrv_pcl_about.cpcN_npics = MAX_PICS;
    }
  TprintfT (DBG_LT1, "hwcdrv: hdrv_pcl_internal_open:"
	    " perf_event cpuver=%d, name='%s'\n",
	    hdrv_pcl_about.cpcN_cpuver, hdrv_pcl_about.cpcN_cciname);
  return 0;

internal_open_error:
  hdrv_pcl_about.cpcN_cpuver = CPUVER_UNDEFINED;
  hdrv_pcl_about.cpcN_npics = 0;
  hdrv_pcl_about.cpcN_docref = NULL;
  hdrv_pcl_about.cpcN_cciname = NULL;
  return HWCFUNCS_ERROR_NOT_SUPPORTED;
}

static void *
single_thread_tsd_ftn ()
{
  static hdrv_pcl_ctx_t tsd_context;
  return &tsd_context;
}

/* HWCDRV_API */
HWCDRV_API int
hwcdrv_init (hwcfuncs_abort_fn_t abort_ftn, int *tsd_sz)
{
  hdrv_pcl_state.find_vpc_ctx = single_thread_tsd_ftn;
  if (tsd_sz)
    *tsd_sz = sizeof (hdrv_pcl_ctx_t);

  if (hdrv_pcl_state.internal_open_called)
    return HWCFUNCS_ERROR_ALREADY_CALLED;
  return hdrv_pcl_internal_open ();
}

HWCDRV_API void
hwcdrv_get_info (int *cpuver, const char **cciname, uint_t *npics,
		 const char **docref, uint64_t *support)
{
  if (cpuver)
    *cpuver = hdrv_pcl_about.cpcN_cpuver;
  if (cciname)
    *cciname = hdrv_pcl_about.cpcN_cciname;
  if (npics)
    *npics = hdrv_pcl_about.cpcN_npics;
  if (docref)
    *docref = hdrv_pcl_about.cpcN_docref;
  if (support)
    *support = HWCFUNCS_SUPPORT_OVERFLOW_PROFILING | HWCFUNCS_SUPPORT_OVERFLOW_CTR_ID;
}

HWCDRV_API int
hwcdrv_enable_mt (hwcfuncs_tsd_get_fn_t tsd_ftn)
{
  if (tsd_ftn)
    hdrv_pcl_state.find_vpc_ctx = tsd_ftn;
  else
    {
      TprintfT (0, "hwcdrv: ERROR: enable_mt(): tsd_ftn==NULL\n");
      return HWCFUNCS_ERROR_UNAVAIL;
    }
  return 0;
}

HWCDRV_API int
hwcdrv_get_descriptions (hwcf_hwc_cb_t *hwc_cb, hwcf_attr_cb_t *attr_cb)
{
  int count = 0;
  if (hwc_cb && hdrv_pcl_state.get_events)
    count = hdrv_pcl_state.get_events (hwc_cb);
  if (attr_cb)
    for (int ii = 0; perfctr_attrs_table && perfctr_attrs_table[ii].attrname; ii++)
      attr_cb (perfctr_attrs_table[ii].attrname);
  if (!count)
    return -1;
  return 0;
}

HWCDRV_API int
hwcdrv_assign_regnos (Hwcentry* entries[], unsigned numctrs)
{
  return hwcdrv_assign_all_regnos (entries, numctrs);
}

static int
internal_hwc_start (int fd)
{
  int rc = ioctl (fd, PERF_EVENT_IOC_REFRESH, 1);
  if (rc == -1)
    {
      TprintfT (DBG_LT0, "hwcdrv: ERROR: internal_hwc_start:"
		" PERF_EVENT_IOC_REFRESH(fd=%d) failed: errno=%d\n", fd, errno);
      return HWCFUNCS_ERROR_UNAVAIL;
    }
  TprintfT (DBG_LT3, "hwcdrv: internal_hwc_start(fd=%d)\n", fd);
  return 0;
}

HWCDRV_API int
hwcdrv_overflow (siginfo_t *si, hwc_event_t *eventp, hwc_event_t *lost_events)
{
  /* set expired counters to overflow value and all others to 0 */
  /* return 0: OK, counters should be restarted */
  /* return non-zero: eventp not set, counters should not be restarted */
  /* clear return values */
  int ii;
  for (ii = 0; ii < hdrv_pcl_state.hwcdef_cnt; ii++)
    {
      eventp->ce_pic[ii] = 0;
      lost_events->ce_pic[ii] = 0;
    }
  hrtime_t sig_ts = gethrtime (); //YXXX get this from HWC event?
  eventp->ce_hrt = sig_ts;
  lost_events->ce_hrt = sig_ts;

  /* determine source signal */
  int signal_fd = -1;
  switch (si->si_code)
    {
    case POLL_HUP: /* expected value from pcl */
      /* According to Stephane Eranian:
       * "expect POLL_HUP instead of POLL_IN because we are
       * in one-shot mode (IOC_REFRESH)"
       */
      signal_fd = si->si_fd;
      break;
    case SI_TKILL: /* event forwarded by tkill */
      /* DBX can only forward SI_TKILL when it detects POLL_HUP
       * unfortunately, this means that si->si_fd has been lost...
       * We need to process the buffers, but we don't know the fd!
       */
      TprintfT (DBG_LT0, "hwcdrv: sig_ts=%llu: WARNING: hwcdrv_overflow:"
		" SI_TKILL detected\n", sig_ts);
      break;
    default:
      // "sometimes we see a POLL_IN (1) with very high event rates,"
      // according to eranian(?)
      TprintfT (DBG_LT0, "hwcdrv: sig_ts=%llu: ERROR: hwcdrv_overflow:"
		" unexpected si_code 0x%x\n", sig_ts, si->si_code);
      return HWCFUNCS_ERROR_GENERIC;
    }

  hdrv_pcl_ctx_t * pctx = hdrv_pcl_state.find_vpc_ctx ();
  if (!pctx)
    {
      TprintfT (DBG_LT0, "hwcdrv: sig_ts=%llu: ERROR: hwcdrv_overflow:"
		" tsd context is NULL\n", sig_ts);
      return HWCFUNCS_ERROR_UNEXPECTED;
    }
  counter_state_t * ctr_list = (counter_state_t *) pctx->ctr_list;
  if (!ctr_list)
    {
      TprintfT (DBG_LT0, "hwcdrv: sig_ts=%llu: WARNING: hwcdrv_overflow:"
		" ctr_list is NULL\n", sig_ts);
      return HWCFUNCS_ERROR_UNEXPECTED;
    }

  /* clear needs_restart flag */
  for (ii = 0; ii < hdrv_pcl_state.hwcdef_cnt; ii++)
    ctr_list[ii].needs_restart = 0;

  /* attempt to identify the counter to read */
  int signal_idx = -1;
  pctx->signal_fd = signal_fd; // save the signal provided by siginfo_t
  if (signal_fd != -1)
    {
      for (ii = 0; ii < hdrv_pcl_state.hwcdef_cnt; ii++)
	{
	  if (ctr_list[ii].fd == signal_fd)
	    {
	      signal_idx = ii;
	      break;
	    }
	}
    }

  if (signal_idx < 0)
    {
      TprintfT (DBG_LT0, "hwcdrv: sig_ts=%llu: ERROR: hwcdrv_overflow:"
		" pmc not determined!\n", sig_ts);
      lost_events->ce_pic[0] = 1; /* record a bogus value into experiment */
      // note: bogus value may get overwritten in loop below
    }

  /* capture sample(s).  In addition to signal_idx, check other counters. */
  struct perf_event_header sheader;
  int idx;
  for (idx = 0; idx < hdrv_pcl_state.hwcdef_cnt; idx++)
    {
      int num_recs = 0;
      while (1)
	{
	  /* check for samples */
	  struct perf_event_mmap_page *metadata = ctr_list[idx].buf_state.buf;
	  if (metadata == NULL)
	    break; // empty
	  if (metadata->data_tail == metadata->data_head)
	    break; // empty

	  /* read header */
	  if (read_buf (&ctr_list[idx].buf_state, &sheader, sizeof (sheader)))
	    break;
	  num_recs++;

	  /* check for PERF_RECORD_SAMPLE */
	  size_t datasz = sheader.size - sizeof (struct perf_event_header);
	  if (sheader.type != PERF_RECORD_SAMPLE)
	    {
	      TprintfT (DBG_LT2, "hwcdrv: sig_ts=%llu: WARNING: hwcdrv_overflow:"
			" unexpected recd type=%d\n",
			sig_ts, sheader.type);
	      if (skip_buf (&ctr_list[idx].buf_state, datasz))
		{
		  TprintfT (DBG_LT0, "hwcdrv: sig_ts=%llu: ERROR: hwcdrv_overflow:"
			    " skip recd type=%d failed\n", sig_ts, sheader.type);
		  lost_events->ce_pic[idx] = 4; /* record a bogus value */
		  break; // failed to skip buffer??
		}
	      lost_events->ce_pic[idx] = 2; /* record a bogus value */
	      continue; // advance to next record
	    }

	  /* type is PERF_RECORD_SAMPLE */
	  uint64_t value, lostv;
	  if (read_sample (&ctr_list[idx], datasz, &value, &lostv))
	    {
	      TprintfT (DBG_LT0, "hwcdrv: sig_ts=%llu: ERROR: hwcdrv_overflow:"
			" read_sample() failed\n", sig_ts);
	      lost_events->ce_pic[idx] = 3; // record a bogus value
	      break;                        // failed to read sample data??
	    }
	  TprintfT (DBG_LT3, "hwcdrv: sig_ts=%llu: hwcdrv_overflow:"
		    " idx=%d value=%llu lost=%llu\n", (unsigned long long) sig_ts,
		    idx, (unsigned long long) value, (unsigned long long) lostv);
	  if (eventp->ce_pic[idx])
	    {
	      TprintfT (DBG_LT2, "hwcdrv: sig_ts=%llu: WARNING: hwcdrv_overflow:"
			" idx=%d previous sample recorded as lost_event\n", sig_ts, idx);
	      lost_events->ce_pic[idx] += eventp->ce_pic[idx];
	    }
	  eventp->ce_pic[idx] = value;
	  lost_events->ce_pic[idx] += lostv;
	}

      /* debug output for unexpected (but common) cases */
      if (idx == signal_idx)
	{
	  if (num_recs != 1)
	    TprintfT (DBG_LT2, "hwcdrv: sig_ts=%llu: WARNING: hwcdrv_overflow:"
		      " %d records for signal_idx=%d\n", sig_ts, num_recs, signal_idx);
	}
      else if (num_recs)
	TprintfT (DBG_LT2, "hwcdrv: sig_ts=%llu: WARNING: hwcdrv_overflow:"
		  " %d unexpected record(s) for idx=%d (signal_idx=%d)\n",
		  sig_ts, num_recs, idx, signal_idx);

      /* trigger counter restart whenever records were found */
      if (num_recs)
	{
	  /* check whether to adapt the overflow interval */
	  /* This is the Linux version.
	   * The Solaris version is in hwprofile.c collector_update_overflow_counters().
	   */
	  hrtime_t min_time = global_perf_event_def[idx].min_time;
	  if (min_time > 0 // overflow interval is adaptive
	      && sig_ts - ctr_list[idx].last_overflow_time < min_time) // last interval below min
	    {
	      /* pick a new overflow interval */
	      /* roughly doubled, but add funny numbers */
	      /* hopefully the result is prime or not a multiple of some # of ops/loop */
	      uint64_t new_period = 2 * ctr_list[idx].last_overflow_period + 37;
#if 0
	      // On Solaris, we report the adjustment to the log file.
	      // On Linux it's hard for us to do so since hwcdrv_pcl.c doesn't know about collector_interface, SP_JCMD_COMMENT, or COL_COMMENT_HWCADJ.
	      // For now we simply don't report.
	      collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">%s %d -> %d</event>\n",
					     SP_JCMD_COMMENT, COL_COMMENT_HWCADJ, global_perf_event_def[idx].name,
					     ctr_list[idx].last_overflow_period, new_period);
#endif
	      /* There are a variety of ways of resetting the period on Linux.
	       * The most elegant is
	       *     ioctl(fd,PERF_EVENT_IOC_PERIOD,&period)
	       * but check the perf_event_open man page for PERF_EVENT_IOC_PERIOD:
	       *     > Prior to Linux 2.6.36 this ioctl always failed due to a bug in the kernel.
	       *     > Prior to Linux 3.14 (or 3.7 on ARM), the new period did not take effect
	       *         until after the next overflow.
	       * So we're kind of stuck shutting the fd down and restarting it with the new period.
	       */
	      if (stop_one_ctr (idx, ctr_list))
		{
		  // EUGENE figure out what to do on error
		}
	      ctr_list[idx].last_overflow_period = new_period;
	      if (start_one_ctr (idx, ctr_list[idx].buf_state.pagesz, pctx, "hwcdrv: ERROR: hwcdrv_overflow (readjust overflow):"))
		{
		  // EUGENE figure out what to do on error
		}
	    }
	  ctr_list[idx].last_overflow_time = sig_ts;
#if 0
	  ctr_list[idx].needs_restart = 1;
#else // seems to be more reliable to restart here instead of hwcdrv_sighlr_restart()
	  internal_hwc_start (ctr_list[idx].fd);
#endif
	}
    }
  return 0; // OK to restart counters
}

HWCDRV_API int
hwcdrv_sighlr_restart (const hwc_event_t *pp)
{
#if 0 // restarting here doesn't seem to work as well as restarting in hwcdrv_overflow()
  hdrv_pcl_ctx_t * pctx = hdrv_pcl_state.find_vpc_ctx ();
  if (!pctx)
    {
      TprintfT (DBG_LT0, "hwcdrv: ERROR: hwcdrv_sighlr_restart: find_vpc_ctx()==NULL\n");
      return -1;
    }
  counter_state_t * ctr_list = (counter_state_t *) pctx->ctr_list;
  if (!ctr_list)
    {
      TprintfT (DBG_LT0, "hwcdrv: WARNING: hwcdrv_sighlr_restart: ctr_list is NULL\n");
      return -1;
    }
  int errors = 0;
  for (int ii = 0; ii < hdrv_pcl_state.hwcdef_cnt; ii++)
    {
      if (ctr_list[ii].needs_restart)
	errors |= internal_hwc_start (ctr_list[ii].fd);
      ctr_list[ii].needs_restart = 0;
    }
  return errors;
#else
  return 0;
#endif
}

/* create counters based on hwcdef[] */
HWCDRV_API int
hwcdrv_create_counters (unsigned hwcdef_cnt, Hwcentry *hwcdef)
{
  if (hwcdef_cnt > hdrv_pcl_about.cpcN_npics)
    {
      logerr (GTXT ("More than %d counters were specified\n"), hdrv_pcl_about.cpcN_npics); /*!*/
      return HWCFUNCS_ERROR_HWCARGS;
    }
  if (hdrv_pcl_about.cpcN_cpuver == CPUVER_UNDEFINED)
    {
      logerr (GTXT ("Processor not supported\n"));
      return HWCFUNCS_ERROR_HWCARGS;
    }

  /* add counters */
  for (unsigned idx = 0; idx < hwcdef_cnt; idx++)
    {
      perf_event_def_t *glb_event_def = &global_perf_event_def[idx];
      memset (glb_event_def, 0, sizeof (perf_event_def_t));
      unsigned int pmc_sel;
      eventsel_t evntsel;
      if (hwcfuncs_get_x86_eventsel (hwcdef[idx].reg_num,
				     hwcdef[idx].int_name, &evntsel, &pmc_sel))
	{
	  TprintfT (0, "hwcdrv: ERROR: hwcfuncs_get_x86_eventsel() failed\n");
	  return HWCFUNCS_ERROR_HWCARGS;
	}
      glb_event_def->reg_num = pmc_sel;
      glb_event_def->eventsel = evntsel;
      glb_event_def->counter_preload = hwcdef[idx].val;
      glb_event_def->min_time = hwcdef[idx].min_time;
      glb_event_def->name = strdup (hwcdef[idx].name); // memory leak??? very minor
      init_perf_event (&glb_event_def->hw, glb_event_def->eventsel,
		       glb_event_def->counter_preload);
      TprintfT (DBG_LT1, "hwcdrv: create_counters: pic=%u name='%s' interval=%lld"
		"(min_time=%lld): reg_num=0x%x eventsel=0x%llx ireset=%lld usr=%lld sys=%lld\n",
		idx, hwcdef[idx].int_name, (long long) glb_event_def->counter_preload,
		(long long) glb_event_def->min_time, (int) glb_event_def->reg_num,
		(long long) glb_event_def->eventsel,
		(long long) HW_INTERVAL_PRESET (hwcdef[idx].val),
		(long long) glb_event_def->hw.exclude_user,
		(long long) glb_event_def->hw.exclude_kernel);
    }

  hdrv_pcl_state.hwcdef_cnt = hwcdef_cnt;
  return 0;
}

HWCDRV_API int
hwcdrv_free_counters () // note: only performs shutdown for this thread
{
  hdrv_pcl_ctx_t * pctx;
  if (!COUNTERS_ENABLED ())
    return 0;
  pctx = hdrv_pcl_state.find_vpc_ctx ();
  if (!pctx)
    {
      TprintfT (0, "hwcdrv: WARNING: hwcdrv_free_counters: tsd context is NULL\n");
      return HWCFUNCS_ERROR_GENERIC;
    }
  counter_state_t *ctr_list = pctx->ctr_list;
  if (!ctr_list)
    {
      // fork child: prolog suspends hwcs, then epilog frees them
      TprintfT (DBG_LT1, "hwcdrv: WARNING: hwcdrv_free_counters: ctr_list is already NULL\n");
      return 0;
    }
  int hwc_rc = 0;
  for (int ii = 0; ii < hdrv_pcl_state.hwcdef_cnt; ii++)
    if (stop_one_ctr (ii, ctr_list))
      hwc_rc = HWCFUNCS_ERROR_GENERIC;
  TprintfT (DBG_LT1, "hwcdrv: hwcdrv_free_counters(tid=0x%lx).\n", (long) pctx->tid);
  pctx->ctr_list = NULL;
  return hwc_rc;
}

HWCDRV_API int
hwcdrv_start (void) /* must be called from each thread ? */
{
  hdrv_pcl_ctx_t *pctx = NULL;
  if (!COUNTERS_ENABLED ())
    {
      TprintfT (DBG_LT1, "hwcdrv: WARNING: hwcdrv_start: no counters to start \n");
      return 0;
    }
  if (!hdrv_pcl_state.library_ok)
    {
      TprintfT (0, "hwcdrv: ERROR: hwcdrv_start: library is not open\n");
      return HWCFUNCS_ERROR_NOT_SUPPORTED;
    }

  /*
   * set up per-thread context
   */
  pctx = hdrv_pcl_state.find_vpc_ctx ();
  if (!pctx)
    {
      TprintfT (0, "hwcdrv: ERROR: hwcdrv_start: tsd context is NULL\n");
      return HWCFUNCS_ERROR_UNEXPECTED;
    }
  pctx->tid = hwcdrv_gettid ();
  TprintfT (DBG_LT1, "hwcdrv: hwcdrv_start(tid=0x%lx)\n", (long) pctx->tid);

  /*
   * create per-thread counter list
   */
  counter_state_t *ctr_list = (counter_state_t *) calloc (hdrv_pcl_state.hwcdef_cnt,
							  sizeof (counter_state_t));
  if (!ctr_list)
    {
      TprintfT (0, "hwcdrv: ERROR: hwcdrv_start: calloc(ctr_list) failed\n");
      return HWCFUNCS_ERROR_MEMORY;
    }
  int ii;
  for (ii = 0; ii < hdrv_pcl_state.hwcdef_cnt; ii++)
    ctr_list[ii].fd = -1; // invalidate fds in case we have to close prematurely
  pctx->ctr_list = ctr_list;

  /*
   * bind the counters
   */
  size_t pgsz = sysconf (_SC_PAGESIZE);
  for (ii = 0; ii < hdrv_pcl_state.hwcdef_cnt; ii++)
    {
      ctr_list[ii].last_overflow_period = global_perf_event_def[ii].hw.sample_period;
      if (start_one_ctr (ii, pgsz, pctx, "hwcdrv: ERROR: hwcdrv_start:")) goto hwcdrv_start_cleanup;
    }

  /*
   * start the counters
   */
  for (ii = 0; ii < hdrv_pcl_state.hwcdef_cnt; ii++)
    {
      int rc = internal_hwc_start (ctr_list[ii].fd);
      if (rc < 0)
	goto hwcdrv_start_cleanup;
    }
  return 0;

hwcdrv_start_cleanup:
  hwcdrv_free_counters (); // PERF_EVENT_IOC_DISABLE and close() for all fds
  return HWCFUNCS_ERROR_UNAVAIL;
}

HWCDRV_API int
hwcdrv_lwp_suspend (void) /* must be called from each thread */
{
  if (!COUNTERS_ENABLED ())
    {
      TprintfT (DBG_LT1, "hwcdrv: WARNING: hwcdrv_lwp_suspend: no counters\n");
      return 0;
    }
  TprintfT (DBG_LT1, "hwcdrv: hwcdrv_lwp_suspend()\n");
  return hwcdrv_free_counters ();
}

HWCDRV_API int
hwcdrv_lwp_resume (void) /* must be called from each thread */
{
  if (!COUNTERS_ENABLED ())
    {
      TprintfT (DBG_LT1, "hwcdrv: WARNING: hwcdrv_lwp_resume: no counters\n");
      return 0;
    }
  TprintfT (DBG_LT1, "hwcdrv: hwcdrv_lwp_resume()\n");
  return hwcdrv_start ();
}

HWCDRV_API int
hwcdrv_read_events (hwc_event_t *overflow_data, hwc_event_samples_t *sampled_data)
{
  overflow_data->ce_hrt = 0;
  for (int i = 0; i < MAX_PICS; i++)
    {
      overflow_data->ce_pic[i] = 0;
      if (sampled_data)
	HWCFUNCS_SAMPLE_RESET (&sampled_data->sample[i]);
    }
  return 0;
}

/*---------------------------------------------------------------------------*/
/* HWCDRV_API */

hwcdrv_api_t hwcdrv_pcl_api = {
  hwcdrv_init,
  hwcdrv_get_info,
  hwcdrv_enable_mt,
  hwcdrv_get_descriptions,
  hwcdrv_assign_regnos,
  hwcdrv_create_counters,
  hwcdrv_start,
  hwcdrv_overflow,
  hwcdrv_read_events,
  hwcdrv_sighlr_restart,
  hwcdrv_lwp_suspend,
  hwcdrv_lwp_resume,
  hwcdrv_free_counters,
  hwcdrv_lwp_init,
  hwcdrv_lwp_fini,
    -1                      // hwcdrv_init_status
};
