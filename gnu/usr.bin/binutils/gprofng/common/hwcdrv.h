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

/* Hardware counter profiling driver's header */

#ifndef __HWCDRV_H
#define __HWCDRV_H

#include "hwcfuncs.h"

#ifdef linux
#define HWCFUNCS_SIGNAL         SIGIO
#define HWCFUNCS_SIGNAL_STRING  "SIGIO"
#else
#define HWCFUNCS_SIGNAL         SIGEMT
#define HWCFUNCS_SIGNAL_STRING  "SIGEMT"
#endif

#ifndef LIBCOLLECTOR_SRC /* not running in libcollector */
#include <string.h>

#else /* running in libcollector */
#include "collector_module.h"
#include "libcol_util.h"

#define get_hwcdrv                  __collector_get_hwcdrv
#define hwcdrv_drivers              __collector_hwcdrv_drivers
#define hwcdrv_cpc1_api             __collector_hwcdrv_cpc1_api
#define hwcdrv_cpc2_api             __collector_hwcdrv_cpc2_api
#define hwcdrv_default              __collector_hwcdrv_default
#define hwcdrv_driver               __collector_hwcdrv_driver
#define hwcdrv_init                 __collector_hwcdrv_init
#define hwcdrv_get_info             __collector_hwcdrv_get_info
#define hwcdrv_enable_mt            __collector_hwcdrv_enable_mt
#define hwcdrv_get_descriptions     __collector_hwcdrv_get_descriptions
#define hwcdrv_assign_regnos        __collector_hwcdrv_assign_regnos
#define hwcdrv_create_counters      __collector_hwcdrv_create_counters
#define hwcdrv_start                __collector_hwcdrv_start
#define hwcdrv_overflow             __collector_hwcdrv_overflow
#define hwcdrv_read_events          __collector_hwcdrv_read_events
#define hwcdrv_sighlr_restart       __collector_hwcdrv_sighlr_restart
#define hwcdrv_lwp_suspend          __collector_hwcdrv_lwp_suspend
#define hwcdrv_lwp_resume           __collector_hwcdrv_lwp_resume
#define hwcdrv_free_counters        __collector_hwcdrv_free_counters
#define hwcdrv_lwp_init             __collector_hwcdrv_lwp_init
#define hwcdrv_lwp_fini             __collector_hwcdrv_lwp_fini
#define hwcdrv_assign_all_regnos    __collector_hwcdrv_assign_all_regnos
#define hwcdrv_lookup_cpuver        __collector_hwcdrv_lookup_cpuver
#define hwcfuncs_int_capture_errmsg  __collector_hwcfuncs_int_capture_errmsg

#define GTXT(x) x

/* Implemented by libcollector */
#define calloc          __collector_calloc
#define close           CALL_UTIL(close)
#define fcntl           CALL_UTIL(fcntl)
#define fprintf         CALL_UTIL(fprintf)
//#define free            __collector_free
#define free(...)
#define gethrtime       __collector_gethrtime
#define ioctl           CALL_UTIL(ioctl)
#define malloc          __collector_malloc
#define memcpy          __collector_memcpy
#define memset          CALL_UTIL(memset)
#define mmap            CALL_UTIL(mmap)
#define snprintf        CALL_UTIL(snprintf)
#define strchr          CALL_UTIL(strchr)
#define strcmp          CALL_UTIL(strcmp)
#define strncmp         CALL_UTIL(strncmp)
#define strcpy          CALL_UTIL(strcpy)
#define strdup          __collector_strdup
#define strncpy         CALL_UTIL(strncpy)
#define strerror        CALL_UTIL(strerror)
#define strlen          CALL_UTIL(strlen)
#define strstr          CALL_UTIL(strstr)
#define strtol          CALL_UTIL(strtol)
#define strtoll         CALL_UTIL(strtoll)
#define strtoul         CALL_UTIL(strtoul)
#define strtoull        CALL_UTIL(strtoull)
#define syscall         CALL_UTIL(syscall)
#define sysconf         CALL_UTIL(sysconf)
#define vsnprintf       CALL_UTIL(vsnprintf)

#endif  /* --- LIBCOLLECTOR_SRC --- */

/* TprintfT(<level>,...) definitions.  Adjust per module as needed */
#define DBG_LT0 0 // for high-level configuration, unexpected errors/warnings
#define DBG_LT1 1 // for configuration details, warnings
#define DBG_LT2 2
#define DBG_LT3 3
#define DBG_LT4 4

#ifdef __cplusplus
extern "C"
{
#endif

  /* hwcdrv api */
  typedef struct
  {
    int (*hwcdrv_init)(hwcfuncs_abort_fn_t abort_ftn, int * tsd_sz);
    /* Initialize hwc counter library (do not call again after fork)
	 Must be called before other functions.
       Input:
	 <abort_ftn>: NULL or callback function to be used for fatal errors
	 <tsd_sz>: If not NULL, returns size in bytes required for thread-specific storage
       Return: 0 if successful
     */

    void (*hwcdrv_get_info)(int *cpuver, const char **cciname, uint_t *npics,
			    const char **docref, uint64_t *support);
    /* get info about session
       Input:
	 <cpuver>: if not NULL, returns value of CPC cpu version
	 <cciname>: if not NULL, returns name of CPU
	 <npics>: if not NULL, returns maximum # of HWCs
	 <docref>: if not NULL, returns documentation reference
	 <support>: if not NULL, returns bitmask (see hwcfuncs.h) of hwc support
       Return: 0 if successful, nonzero otherwise
     */

    int (*hwcdrv_enable_mt)(hwcfuncs_tsd_get_fn_t tsd_ftn);
    /* Enables multi-threaded mode (do not need to call again after fork)
       Input:
	 <tsd_ftn>: If <tsd_sz>==0, this parameter is ignored.
		    Otherwise:
		     tsd_ftn() must be able to return a pointer to thread-specific
		     memory of <tsd_sz> bytes.
		     For a given thread, tsd_ftn() must
		     always return the same pointer.
       Return: none
     */

    int (*hwcdrv_get_descriptions)(hwcf_hwc_cb_t *hwc_find_action,
				   hwcf_attr_cb_t *attr_find_action);
    /* Initiate callbacks with all available HWC names and and HWC attributes.
       Input:
	 <hwc_find_action>: if not NULL, will be called once for each HWC
	 <attr_find_action>: if not NULL, will be called once for each attribute
       Return: 0 if successful
	  or a cpc return code upon error
     */

    int (*hwcdrv_assign_regnos)(Hwcentry* entries[], unsigned numctrs);
    /* Assign entries[]->reg_num values as needed by platform
       Input:
	 <entries>: array of counters
	 <numctrs>: number of items in <entries>
       Return: 0 if successful
	  HWCFUNCS_ERROR_HWCINIT if resources unavailable
	  HWCFUNCS_ERROR_HWCARGS if counters were not specified correctly
     */

    int (*hwcdrv_create_counters)(unsigned hwcdef_cnt, Hwcentry *hwcdef);
    /* Create the counters, but don't start them.
	 call this once in main thread to create counters.
       Input:
	 <defcnt>: number of counter definitions.
	 <hwcdef>: counter definitions.
       Return: 0 if successful
	  or a cpc return code upon error
     */

    int (*hwcdrv_start)(void);
    /* Start the counters.
	 call this once in main thread to start counters.
       Return: 0 if successful
	  or a cpc return code upon error
     */

    int (*hwcdrv_overflow)(siginfo_t *si, hwc_event_t *sample,
			   hwc_event_t *lost_samples);
    /* Linux only.  Capture current counter values.
	 This is intended to be called from SIGEMT handler;
       Input:
	 <si>: signal handler context information
	 <sample>: returns non-zero values for counters that overflowed
	 <lost_samples>: returns non-zero values for counters that "lost" counts
       Return: 0 if successful
	  or a cpc return code upon error.
     */

    int (*hwcdrv_read_events)(hwc_event_t *overflow_data,
			      hwc_event_samples_t *sampled_data);
    /* Read current counter values and samples.  Read of samples is destructive.
       Note: hwcdrv_read_events is not supported on Linux.
       <overflow_data>: returns snapshot of counter values
       <sampled_data>: returns sampled data
       Return: 0 if successful
	  HWCFUNCS_ERROR_UNAVAIL if resource unavailable(e.g. called before initted)
	  (other values may be possible)
     */

    int (*hwcdrv_sighlr_restart)(const hwc_event_t* startVals);
    /* Restarts the counters at the given value.
	 This is intended to be called from SIGEMT handler;
       Input:
	 <startVals>: Solaris: new start values.
		      Linux: pointer may be NULL; startVals is ignored.
       Return: 0 if successful
	  or a cpc return code upon error.
     */

    int (*hwcdrv_lwp_suspend)(void);
    /* Attempt to stop counters on this lwp only.
	 hwcdrv_lwp_resume() should be used to restart counters.
       Return: 0 if successful
	  or a cpc return code upon error.
     */

    int (*hwcdrv_lwp_resume)(void);
    /* Attempt to restart counters on this lwp when counters were
	 stopped with hwcdrv_lwp_suspend().
       Return: 0 if successful
	  or a cpc return code upon error.
     */

    int (*hwcdrv_free_counters)(void);
    /* Stops counters on this lwp only and frees resources.
	 This will fail w/ unpredictable results if other lwps's are
	 still running.  After this call returns,
	 hwcdrv_create_counters() may be called with new values.
       Return: 0 if successful
	  or a cpc return code upon error.
     */

    int (*hwcdrv_lwp_init)(void);
    /* per-thread counter init.
	 Solaris: nop.
	 Linux: just after thread creation call this from inside thread
	      to create context and start counters.
       Return: 0 if successful
	  or a perfctr return code upon error
     */

    void (*hwcdrv_lwp_fini)(void);
    /* per-thread counter cleanup.
	 Solaris: nop.
	 Linux: call in each thread upon thread destruction.
     */

    int hwcdrv_init_status;
  } hwcdrv_api_t;

  extern hwcdrv_api_t *get_hwcdrv ();
  extern hwcdrv_api_t *__collector_get_hwcdrv ();
  extern int __collector_hwcfuncs_bind_descriptor (const char *defstring);
  extern Hwcentry **__collector_hwcfuncs_get_ctrs (unsigned *defcnt);
  extern hwcdrv_api_t *hwcdrv_drivers[]; // array of available drivers

  /* prototypes for internal use by hwcdrv drivers */
  typedef struct
  { // see hwcdrv_get_info() for field definitions
    int cpcN_cpuver;
    uint_t cpcN_npics;
    const char *cpcN_docref;
    const char *cpcN_cciname;
  } hwcdrv_about_t;

  extern int hwcdrv_assign_all_regnos (Hwcentry* entries[], unsigned numctrs);
  /* assign user's counters to specific CPU registers */

  extern int hwcdrv_lookup_cpuver (const char * cpcN_cciname);
  /* returns hwc_cpus.h ID for a given string. */

  extern void hwcfuncs_int_capture_errmsg (const char *fn, int subcode,
					   const char *fmt, va_list ap);
#define logerr  hwcfuncs_int_logerr

  /*---------------------------------------------------------------------------*/
  /* prototypes for internal use by linux hwcdrv drivers */
#define PERFCTR_FIXED_MAGIC 0x40000000 /* tells perfctr to use intel fixed pmcs */
#define PERFCTR_UMASK_SHIFT 8
#define EXTENDED_EVNUM_2_EVSEL(evnum) \
  ( (((eventsel_t)(evnum) & 0x0f00ULL) << 24) | ((eventsel_t)(evnum) & ~0x0f00ULL) )

  typedef uint64_t eventsel_t;
  extern int  hwcfuncs_get_x86_eventsel (unsigned int regno, const char *int_name,
			     eventsel_t *return_event, uint_t *return_pmc_sel);

  typedef int (hwcdrv_get_events_fn_t) (hwcf_hwc_cb_t *hwc_cb);
  typedef int (hwcdrv_get_eventnum_fn_t) (const char *eventname, uint_t pmc,
					  eventsel_t *eventnum,
					  eventsel_t *valid_umask, uint_t *pmc_sel);
  extern hwcdrv_get_eventnum_fn_t *hwcdrv_get_x86_eventnum;

  typedef struct
  {
    const char * attrname;  // user-visible name of attribute
    int is_inverted;        // nonzero means boolean attribute is inverted
    eventsel_t mask;        // which attribute bits can be set?
    eventsel_t shift;       // how far to shift bits for use in x86 register
  } attr_info_t;
  extern const attr_info_t *perfctr_attrs_table;

  /* hdrv_pcbe api: cpu-specific drivers for Linux */
  typedef struct
  {
    int (*hdrv_pcbe_init)(void);
    uint_t (*hdrv_pcbe_ncounters)(void);
    const char *(*hdrv_pcbe_impl_name)(void);
    const char *(*hdrv_pcbe_cpuref)(void);
    int (*hdrv_pcbe_get_events)(hwcf_hwc_cb_t *hwc_cb);
    int (*hdrv_pcbe_get_eventnum)(const char * eventname, uint_t pmc,
				  eventsel_t *eventnum, eventsel_t *valid_umask,
				  uint_t *pmc_sel);
  } hdrv_pcbe_api_t;

#ifdef __cplusplus
}
#endif

#endif
