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

#ifndef _EXP_LAYOUT_H
#define _EXP_LAYOUT_H

#include <sys/time.h>
#include <sys/types.h>

#include "dbe_types.h"
#include "gp-experiment.h"
#include "data_pckts.h"
#include "ABS.h"
#include "Data_window.h"
#include "Histable.h"
#include "vec.h"

class PrUsage
{
public:
  PrUsage ();
  PrUsage *bind32 (void *p, bool need_swap_endian);
  PrUsage *bind64 (void *p, bool need_swap_endian);
  static uint64_t bind32Size ();
  static uint64_t bind64Size ();
  Vector<long long> * getMstateValues ();

  hrtime_t pr_tstamp;
  hrtime_t pr_create;
  hrtime_t pr_term;
  hrtime_t pr_rtime;

  // the following correspond to PROP_MSTATE LMS_* offsets; see newMstateVec()
  hrtime_t pr_utime;
  hrtime_t pr_stime;
  hrtime_t pr_ttime;
  hrtime_t pr_tftime;
  hrtime_t pr_dftime;
  hrtime_t pr_kftime;
  hrtime_t pr_ltime;
  hrtime_t pr_slptime;
  hrtime_t pr_wtime;
  hrtime_t pr_stoptime;

  uint64_t pr_minf;
  uint64_t pr_majf;
  uint64_t pr_nswap;
  uint64_t pr_inblk;
  uint64_t pr_oublk;
  uint64_t pr_msnd;
  uint64_t pr_mrcv;
  uint64_t pr_sigs;
  uint64_t pr_vctx;
  uint64_t pr_ictx;
  uint64_t pr_sysc;
  uint64_t pr_ioch;
};

class DataView;
extern void *getStack (VMode, DataView*, long);
extern int stackSize (VMode, DataView*, long);
extern Histable *getStackPC (int, VMode, DataView*, long);
extern Vector<Histable*> *getStackPCs (VMode, DataView*, long);

class CommonPacket // use only for RacePacket, please
{
public:
  CommonPacket ();
  void *getStack (VMode);
  Histable *getStackPC (int, VMode);
  Vector<Histable*>*getStackPCs (VMode);
  static int cmp (const void *a, const void *b);

  enum Tag_type  { LWP, THR, CPU };
  static const int NTAGS = 3;
  uint32_t tags[NTAGS];         // lwp_id, thr_id, cpu_id
  hrtime_t tstamp;
  struct JThread *jthread_TBR;  // pointer to JThread or NULL
  uint64_t frinfo;              // frame info
  Vaddr leafpc;                 // raw leaf PC if availabe
  void *nat_stack;              // native stack
  void *user_stack;             // user stack (Java, OMP, etc.)
  static void *jvm_overhead;
};

class FramePacket
{
public:
  int
  stackSize (bool java = false)
  {
    return java ? jstack->size () / 2 : stack->size ();
  }

  Vaddr
  getFromStack (int n)
  {
    return stack->fetch (n);
  }

  Vaddr
  getMthdFromStack (int n)
  {
    return jstack->fetch (2 * n + 1);
  }

  int
  getBciFromStack (int n)
  {
    return (int) jstack->fetch (2 * n);
  }

  bool
  isLeafMark (int n)
  {
    return stack->fetch (n) == (Vaddr) SP_LEAF_CHECK_MARKER;
  }

  bool
  isTruncatedStack (bool java = false)
  {
    return java ? jtruncated : truncated == (Vaddr) SP_TRUNC_STACK_MARKER;
  }

  bool
  isFailedUnwindStack ()
  {
    return truncated == (Vaddr) SP_FAILED_UNWIND_MARKER;
  }
  uint32_t omp_state; // OpenMP thread state
  uint32_t mpi_state; // MPI state
  uint64_t omp_cprid; // OpenMP parallel region id (omptrace)
  Vector<Vaddr> *stack;
  Vaddr truncated;
  Vector<Vaddr> *jstack;
  bool jtruncated;
  Vector<Vaddr> *ompstack;
  Vaddr omptruncated;
};

#endif /* _EXP_LAYOUT_H */
