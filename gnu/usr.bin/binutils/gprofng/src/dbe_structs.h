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

#ifndef _DBE_STRUCTS_H
#define _DBE_STRUCTS_H

#include "dbe_types.h"
#include "enums.h"

typedef enum
{
  Sp_lang_unknown   = 0,
  Sp_lang_asm       = 1,
  Sp_lang_c         = 2,
  Sp_lang_ansic     = 3,
  Sp_lang_cplusplus = 4,
  Sp_lang_fortran   = 5,
  Sp_lang_pascal    = 6,
  Sp_lang_fortran90 = 7,
  Sp_lang_java      = 8,
  Sp_lang_c99       = 9,
  Sp_lang_gcc       = 16,
  Sp_lang_KAI_KPTS  = 32,
  Sp_lang_KAI_KCC   = 33,
  Sp_lang_KAI_Kcc   = 34
} Sp_lang_code;

struct Value
{
  union
  {
    short s;
    int i;
    float f;
    double d;
    timestruc_t t;
    char *l;                // Label
    unsigned long long ll;  // address
  };
};

// sync enum changes with both AnMetric.java and AnVariable.java
enum ValueTag
{
  VT_SHORT = 1,
  VT_INT,
  VT_LLONG,
  VT_FLOAT,
  VT_DOUBLE,
  VT_HRTIME,
  VT_LABEL,
  VT_ADDRESS,
  VT_OFFSET,
  VT_ULLONG
};

// Tagged numeric value
struct TValue
{
  ValueTag tag;
  bool sign;    // The print result will always begin with a sign (+ or -).
  union
  {
    short s;
    int i;
    float f;
    double d;
    char *l;
    void *p;
    long long ll;
    unsigned long long ull;
  };
  double to_double ();
  int to_int ();
  char *to_str (char *str, size_t strsz);
  size_t get_len ();
  void make_delta (TValue *v1, TValue *v2);
  void make_ratio (TValue *v1, TValue *v2);
  int compare (TValue *v);
};

// XXX MAX_HWCOUNT may need to be managed dynamically, not #defined
#define MAX_HWCOUNT 64

// Experiment collection parameters
struct Collection_params
{
  int profile_mode;     // if clock-profiling is on
  long long ptimer_usec; // Clock profile timer interval (microseconds)
  int lms_magic_id;     // identifies which LMS_* states are live
  int sync_mode;        // if synctrace is on
  int sync_threshold;   // value of synctrace threshold, in microseconds
  int sync_scope;       // value of synctrace scope: Java and/or native

  int heap_mode;        // if heaptrace is on
  int io_mode;          // if iotrace is on
  int race_mode;        // if race-detection is on
  int race_stack;       // setting for stack data collection
  int deadlock_mode;    // if deadlock-detection is on
  int omp_mode;         // if omptrace is on

  int hw_mode;          // if hw-counter profiling is on
  int xhw_mode;    // if extended (true-PC) HW counter profiling for any counter

  char *hw_aux_name[MAX_HWCOUNT];
  char *hw_username[MAX_HWCOUNT];
  int hw_interval[MAX_HWCOUNT];     // nominal interval for count
  int hw_tpc[MAX_HWCOUNT];          // non-zero, if aggressive TPC/VA requested
  int hw_metric_tag[MAX_HWCOUNT];   // tag as used for finding metrics
  int hw_cpu_ver[MAX_HWCOUNT];      // Chip version number for this metric

  int sample_periodic;      // if periodic sampling is on
  int sample_timer;         // Sample timer (sec)
  int limit;                // experiment size limit
  const char *pause_sig;    // Pause/resume signal string
  const char *sample_sig;   // Sampling signal string
  const char *start_delay;  // Data collect start delay string
  const char *terminate;    // Data collection termination time string
  char *linetrace;
};

const hrtime_t ZERO_TIME = (hrtime_t) 0;
const hrtime_t MAX_TIME = (hrtime_t) 0x7fffffffffffffffLL;

#define PCInvlFlag              ((int) 0x8LL)
#define PCLineFlag              ((int) 0x4LL)
#define PCTrgtFlag              ((int) 0x2LL)
#define MAKE_ADDRESS(idx, off)  (((unsigned long long)(idx)<<32) | off)
#define ADDRESS_SEG(x)          ((unsigned int)(((x)>>32) & 0xffffffff))
#define ADDRESS_OFF(x)          ((unsigned int)((x) & 0xffffffff))

//
//	Analyzer info
#define AnalyzerInfoVersion 2

typedef struct
{
  uint64_t text_labelref;
  int32_t entries;
  uint32_t version;
} AnalyzerInfoHdr;      // => header from .__analyzer_info

typedef struct
{
  uint32_t offset;      // offset relative to text_labelref
  uint32_t id;          // profiled instruction identifier
  uint32_t signature;   // signature of profiled instruction
  uint32_t datatype_id; // referenced datatype identifier
} memop_info_t;         // => used for table_type=0,1,2

typedef struct
{
  uint32_t offset;      // offset relative to text_labelref
} target_info_t;        // => used for table_type=3

typedef struct
{
  uint32_t type;
  uint32_t offset;
  union
  {
    memop_info_t *memop;
    target_info_t *target;
  };
} inst_info_t;

class DataObject;

typedef struct
{
  uint32_t datatype_id; // datatype identifier (local)
  uint32_t memop_refs;  // count of referencing memops
  uint32_t event_data;  // count of event data
  DataObject *dobj;     // corresponding dataobject (unique)
} datatype_t;

typedef struct
{
  uint32_t offset;      // entry offset in compilation unit
  uint32_t extent;      // sibling offset
  void *parent;         // container symbol
  void *object;         // resolved object
} symbol_t;

typedef struct
{
  char *old_prefix;
  char *new_prefix;
} pathmap_t;

typedef struct
{
  char *libname;
  enum LibExpand expand;
} lo_expand_t;

typedef struct
{
  int index1;
  int index2;
} int_pair_t;
#endif /* _DBE_STRUCTS_H */
