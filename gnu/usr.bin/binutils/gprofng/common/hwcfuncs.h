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

/* Hardware counter profiling */

#ifndef __HWCFUNCS_H
#define __HWCFUNCS_H

#ifdef LIBCOLLECTOR_SRC /* running in libcollector */
#define hwcfuncs_int_logerr         __collector_hwcfuncs_int_logerr
#define hwcfuncs_parse_ctr          __collector_hwcfuncs_parse_ctr
#define hwcfuncs_parse_attrs        __collector_hwcfuncs_parse_attrs
#define hwcfuncs_bind_descriptor    __collector_hwcfuncs_bind_descriptor
#define hwcfuncs_bind_hwcentry      __collector_hwcfuncs_bind_hwcentry
#define hwcfuncs_assign_regnos      __collector_hwcfuncs_assign_regnos
#define regno_is_valid              __collector_regno_is_valid
#define hwcfuncs_get_ctrs           __collector_hwcfuncs_get_ctrs
#define hwcfuncs_errmsg_get         __collector_hwcfuncs_errmsg_get
#endif  /* --- LIBCOLLECTOR_SRC --- */

#include <signal.h>     /* siginfo_t */
#include <limits.h>     /* UINT64_t */
#include <sys/types.h>
#include <stdint.h>

#include "hwcentry.h"   /* for Hwcentry type */
#include "gp-time.h"

typedef unsigned int uint_t;

#ifdef	__cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/* compile options */

#define HWC_DEBUG   0 /* 0/1 to enable extra HWC debug */

/*---------------------------------------------------------------------------*/
/* typedefs */
/* generic hw event */
  typedef struct _hwc_event_t
  { /* generalized counter event */
    hrtime_t ce_hrt;            /* gethrtime() */
    uint64_t ce_pic[MAX_PICS];  /* counter samples or start values */
  } hwc_event_t;

  /* supplementary data that accompanies some hw events */
  typedef struct
  { /* supplementary data fields */
    uint64_t smpl_pc;           /* pc related to event */
    uint64_t smpl_data_source;  /* chip-specific data source encoding */
    uint64_t smpl_latency;      /* latency related to event */
    uint64_t smpl_mem_addr;     /* memory address related to event */
  } hwc_sample_t;
#define HWCFUNCS_INVALID_U64 0xFEEDBEEFDEADBEEFllu /* identifies fields as unused */

typedef struct {                                /* supplementary data fields */
    hwc_sample_t sample[MAX_PICS];  /* counter samples or start values */
} hwc_event_samples_t;

#define HWCFUNCS_SAMPLE_RESET(sample) \
	do { \
	    (sample)->smpl_pc          =HWCFUNCS_INVALID_U64; \
	    (sample)->smpl_data_source =HWCFUNCS_INVALID_U64; \
	    (sample)->smpl_latency     =HWCFUNCS_INVALID_U64; \
	    (sample)->smpl_mem_addr    =HWCFUNCS_INVALID_U64; \
	} while(0)

#define HWCFUNCS_SAMPLE_IS_RESET(sample) \
	( \
	    (sample)->smpl_pc         ==HWCFUNCS_INVALID_U64 && \
	    (sample)->smpl_data_source==HWCFUNCS_INVALID_U64 && \
	    (sample)->smpl_latency    ==HWCFUNCS_INVALID_U64 && \
	    (sample)->smpl_mem_addr   ==HWCFUNCS_INVALID_U64 \
	)

/*---------------------------------------------------------------------------*/
/* macros */

#define HW_INTERVAL_MAX         UINT64_MAX
#define HW_INTERVAL_PRESET(x)   (HW_INTERVAL_MAX - ((uint64_t)(x) - 1))
#define HW_INTERVAL_TYPE(x)     ((uint64_t) (x)

/* parsing */
#define HWCFUNCS_MAX_ATTRS              20
#define HWCFUNCS_PARSE_ATTR             '~'
#define HWCFUNCS_PARSE_EQUAL            '='
#define HWCFUNCS_PARSE_BACKTRACK        '+'
#define HWCFUNCS_PARSE_BACKTRACK_OFF    '-'
#define HWCFUNCS_PARSE_REGNUM           '/'
#define HWCFUNCS_PARSE_VALUE            ','

/* error codes */
#define HWCFUNCS_ERROR_GENERIC          (-1)
#define HWCFUNCS_ERROR_NOT_SUPPORTED    (-2)
#define HWCFUNCS_ERROR_ALREADY_CALLED   (-3)
#define HWCFUNCS_ERROR_HWCINIT          (-4)
#define HWCFUNCS_ERROR_HWCARGS          (-5)
#define HWCFUNCS_ERROR_MEMORY           (-6)
#define HWCFUNCS_ERROR_UNAVAIL          (-7)
#define HWCFUNCS_ERROR_ERRNO_ZERO       (-8)
#define HWCFUNCS_ERROR_UNEXPECTED       (-99)

/*---------------------------------------------------------------------------*/
/* prototypes */

typedef void (*hwcfuncs_abort_fn_t) (int errnum, const char *msg);

extern void hwcfuncs_int_logerr(const char *format,...);
/* Log an error to the internal error buffer. See hwcfuncs_errmsg_get().
     Note: Not MT-safe; don't even enable logging in an MT environment.
       Recommend using this call only during init.
     Note: when a libcpc call fails, it may automatically call
       cpcN_capture_errmsg() to log the error message in the same internal buffer.
       Recommend using this call only for non-cpc failures.
 */

#define HWCFUNCS_SUPPORT_OVERFLOW_PROFILING 0x01llu
#define HWCFUNCS_SUPPORT_PEBS_SAMPLING      0x02llu
#define HWCFUNCS_SUPPORT_OVERFLOW_CTR_ID    0x04llu // OS identifies which counter overflowed
  /* get info about session
     Input:
       <cpuver>: if not NULL, returns value of CPC cpu version
       <cciname>: if not NULL, returns name of CPU
       <npics>: if not NULL, returns maximum # of HWCs
       <docref>: if not NULL, returns documentation reference
       <support>: if not NULL, returns bitmask (see above) of hwc support
     Return: none
   */

  typedef void* (*hwcfuncs_tsd_get_fn_t) (void);
  typedef void (hwcf_hwc_cb_t) (uint_t cpcregno, const char *name);
  typedef void (hwcf_attr_cb_t) (const char *attr);

  extern void
  hwcfuncs_parse_ctr (const char *counter_def, int *pplus, char **pnameOnly,
		      char **pattrs, char **pregstr, regno_t *pregno);
/* Parse a counter definition string (value must already be stripped off).
     Input:
       <counter_def>: input whose format is
	 [+|-]<countername>[~attrs...][/<regno>]
       pointers to return values:  Any can be NULL.
     Return:
       <plus>: 1 if [+] is found, -1 if [-] is found, 0 otherwise
       <pnameonly>: strdup(<countername>)
       <pattrs>: strdup([~attrs...]) if specified, NULL otherwise.
       <pregstr>: strdup(/<regno>) if specified, NULL otherwise.
       <pregno>: <regno> if readable, REGNO_ANY if not specd, or -2 otherwise.
   */

  typedef struct
  {
    char *ca_name;
    uint64_t ca_val;
  } hwcfuncs_attr_t; /* matches cpc_attr_t */

  void * hwcfuncs_parse_attrs (const char *countername,
			       hwcfuncs_attr_t attrs[], unsigned max_attrs,
			       uint_t *pnum_attrs, char **errstring);
  /* Extract the attribute fields from <countername>.
       Input:
	 <countername>: string whose format is
	    [+]<ctrname>[~attributes...][/<regno>][,...]
	 <attrs>: array of attributes to be returned
	 <max_attrs>: number of elements in <attrs>
	 <pnum_attrs>: if not NULL, will return how many attrs were found.
	 <errstring>: pointer to a buffer for storing error info, or NULL.
       Return: upon success, a pointer to an allocated copy of <countername>, or
	   NULL if there's a failure.  (A copy is made in order to provide storage
	   for the ca_name fields in the <attrs> array.)

	   The pointer should be freed when <attrs> is no longer in use.
	   <attrs> will be filled in data from countername.
	 <pnum_attrs> will have the number of elements in <attrs>.  May be
	   non-zero even if return value indicates an error.
	 <errstring> NULL if no error, otherwise, a malloc'd GTXT string.
   */

  extern int hwcfuncs_bind_descriptor (const char *defstring);
  /* Bind counters to resources.
     Input:
       <defstring>: string whose format is
	  :%s:%s:0x%x:%d:%d,0x%x[:%s...repeat for each ctr]
	 where the fields are:
	  :<userName>:<internalCtr>:<register>:<timeoutVal>:<tag>:<memop>
     Return: 0 if successful
	HWCFUNCS_ERROR_HWCINIT if resources unavailable
	HWCFUNCS_ERROR_HWCARGS if counters were not specified correctly
   */

  extern int hwcfuncs_bind_hwcentry (const Hwcentry *entries[],
				     unsigned numctrs);
  /* Bind counters to resources.
     Input:
       <entries>: array of counters
       <numctrs>: number of items in <entries>
     Return: 0 if successful
	HWCFUNCS_ERROR_HWCINIT if resources unavailable
	HWCFUNCS_ERROR_HWCARGS if counters were not specified correctly
   */

  extern int hwcfuncs_assign_regnos (Hwcentry *entries[], unsigned numctrs);
  /* Assign entries[]->reg_num values as needed by platform
       Note: modifies <entries> by supplying a regno to each counter
     Input:
       <entries>: array of counters
       <numctrs>: number of items in <entries>
     Output:
       <entries>: array of counters is modified
     Return: 0 if successful
	HWCFUNCS_ERROR_HWCINIT if resources unavailable
	HWCFUNCS_ERROR_HWCARGS if counters were not specified correctly
   */

  extern int regno_is_valid (const Hwcentry *pctr, regno_t regno);
  /* return 1 if <regno> is in Hwcentry's list
     Input:
       <pctr>: counter definition, reg_list[] should be initialized
       <regno>: register to check
     Return: 1 if <regno> is in Hwcentry's list, 0 otherwise
   */

  extern Hwcentry **hwcfuncs_get_ctrs (unsigned *defcnt);
  /* Get descriptions of the currently bound counters.
     Input:
       <defcnt>: if not NULL, returns number of counter definitions.
     Return:
       table of counter definition pointers
   */

  extern char *hwcfuncs_errmsg_get (char * buf, size_t bufsize,
				    int enable_capture);
  /* Gets a recent HWC error message.
       To clear previous error messages and insure error message is enabled,
       call hwcfuncs_errmsg_get(NULL,0,1).
       Once enabled, one error is stored in an internal buffer.  A call to this
       function will clear the buffer and allow a new message to be captured.
       Note: Not MT-safe - don't enable this feature in an MT environment.
     Input:
       <buf>: pointer to buffer or NULL.
       <bufsize>: size of <buf>
       <enable_capture>: 0 - disable buffering, 1 - enable buffering.
     Return: error string or an empty string.
   */

#ifdef	__cplusplus
}
#endif

#endif /* ! __HWCFUNCS_H */
