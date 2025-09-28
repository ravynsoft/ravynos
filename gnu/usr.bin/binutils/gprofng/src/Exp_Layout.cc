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

#include "config.h"
#include "CallStack.h"
#include "DbeSession.h"
#include "Exp_Layout.h"
#include "Experiment.h"
#include "Function.h"
#include "Table.h"
#include "dbe_types.h"
#include "util.h"

/*
 * PrUsage is a class which wraps access to the values of prusage
 * system structure. It was expanded to 64 bit entities in 2.7
 * (experiment version 6 & 7).
 */
PrUsage::PrUsage ()
{
  pr_tstamp = pr_create = pr_term = pr_rtime = (hrtime_t) 0;
  pr_utime = pr_stime = pr_ttime = pr_tftime = pr_dftime = (hrtime_t) 0;
  pr_kftime = pr_ltime = pr_slptime = pr_wtime = pr_stoptime = (hrtime_t) 0;

  pr_minf = pr_majf = pr_nswap = pr_inblk = pr_oublk = 0;
  pr_msnd = pr_mrcv = pr_sigs = pr_vctx = pr_ictx = pr_sysc = pr_ioch = 0;
}

/*
 * Resource usage.  /proc/<pid>/usage /proc/<pid>/lwp/<lwpid>/lwpusage
 */
struct timestruc_32
{ /* v8 timestruc_t */
  uint32_t tv_sec;              /* seconds */
  uint32_t tv_nsec;             /* and nanoseconds */
};

typedef struct ana_prusage
{
  id_t pr_lwpid;                /* lwp id.  0: process or defunct */
  int pr_count;                 /* number of contributing lwps */
  timestruc_32 pr_tstamp;       /* current time stamp */
  timestruc_32 pr_create;       /* process/lwp creation time stamp */
  timestruc_32 pr_term;         /* process/lwp termination time stamp */
  timestruc_32 pr_rtime;        /* total lwp real (elapsed) time */
  timestruc_32 pr_utime;        /* user level cpu time */
  timestruc_32 pr_stime;        /* system call cpu time */
  timestruc_32 pr_ttime;        /* other system trap cpu time */
  timestruc_32 pr_tftime;       /* text page fault sleep time */
  timestruc_32 pr_dftime;       /* data page fault sleep time */
  timestruc_32 pr_kftime;       /* kernel page fault sleep time */
  timestruc_32 pr_ltime;        /* user lock wait sleep time */
  timestruc_32 pr_slptime;      /* all other sleep time */
  timestruc_32 pr_wtime;        /* wait-cpu (latency) time */
  timestruc_32 pr_stoptime;     /* stopped time */
  timestruc_32 filltime[6];     /* filler for future expansion */
  uint32_t pr_minf;             /* minor page faults */
  uint32_t pr_majf;             /* major page faults */
  uint32_t pr_nswap;            /* swaps */
  uint32_t pr_inblk;            /* input blocks */
  uint32_t pr_oublk;            /* output blocks */
  uint32_t pr_msnd;             /* messages sent */
  uint32_t pr_mrcv;             /* messages received */
  uint32_t pr_sigs;             /* signals received */
  uint32_t pr_vctx;             /* voluntary context switches */
  uint32_t pr_ictx;             /* involuntary context switches */
  uint32_t pr_sysc;             /* system calls */
  uint32_t pr_ioch;             /* chars read and written */
  uint32_t filler[10];          /* filler for future expansion */
} raw_prusage_32;

uint64_t
PrUsage::bind32Size ()
{
  uint64_t bindSize = sizeof (raw_prusage_32);
  return bindSize;
}

#define timestruc2hr(x) ((hrtime_t)(x).tv_sec*NANOSEC + (hrtime_t)(x).tv_nsec)

PrUsage *
PrUsage::bind32 (void *p, bool need_swap_endian)
{
  if (p == NULL)
    return NULL;
  raw_prusage_32 pu, *tmp = (raw_prusage_32*) p;
  if (need_swap_endian)
    {
      pu = *tmp;
      tmp = &pu;
      SWAP_ENDIAN (pu.pr_tstamp.tv_sec);
      SWAP_ENDIAN (pu.pr_tstamp.tv_nsec);
      SWAP_ENDIAN (pu.pr_create.tv_sec);
      SWAP_ENDIAN (pu.pr_create.tv_nsec);
      SWAP_ENDIAN (pu.pr_term.tv_sec);
      SWAP_ENDIAN (pu.pr_term.tv_nsec);
      SWAP_ENDIAN (pu.pr_rtime.tv_sec);
      SWAP_ENDIAN (pu.pr_rtime.tv_nsec);
      SWAP_ENDIAN (pu.pr_utime.tv_sec);
      SWAP_ENDIAN (pu.pr_utime.tv_nsec);
      SWAP_ENDIAN (pu.pr_stime.tv_sec);
      SWAP_ENDIAN (pu.pr_stime.tv_nsec);
      SWAP_ENDIAN (pu.pr_ttime.tv_sec);
      SWAP_ENDIAN (pu.pr_ttime.tv_nsec);
      SWAP_ENDIAN (pu.pr_tftime.tv_sec);
      SWAP_ENDIAN (pu.pr_tftime.tv_nsec);
      SWAP_ENDIAN (pu.pr_dftime.tv_sec);
      SWAP_ENDIAN (pu.pr_dftime.tv_nsec);
      SWAP_ENDIAN (pu.pr_kftime.tv_sec);
      SWAP_ENDIAN (pu.pr_kftime.tv_nsec);
      SWAP_ENDIAN (pu.pr_ltime.tv_sec);
      SWAP_ENDIAN (pu.pr_ltime.tv_nsec);
      SWAP_ENDIAN (pu.pr_slptime.tv_sec);
      SWAP_ENDIAN (pu.pr_slptime.tv_nsec);
      SWAP_ENDIAN (pu.pr_wtime.tv_sec);
      SWAP_ENDIAN (pu.pr_wtime.tv_nsec);
      SWAP_ENDIAN (pu.pr_stoptime.tv_sec);
      SWAP_ENDIAN (pu.pr_stoptime.tv_nsec);
      SWAP_ENDIAN (pu.pr_minf);
      SWAP_ENDIAN (pu.pr_majf);
      SWAP_ENDIAN (pu.pr_nswap);
      SWAP_ENDIAN (pu.pr_inblk);
      SWAP_ENDIAN (pu.pr_oublk);
      SWAP_ENDIAN (pu.pr_msnd);
      SWAP_ENDIAN (pu.pr_mrcv);
      SWAP_ENDIAN (pu.pr_sigs);
      SWAP_ENDIAN (pu.pr_vctx);
      SWAP_ENDIAN (pu.pr_ictx);
      SWAP_ENDIAN (pu.pr_sysc);
      SWAP_ENDIAN (pu.pr_ioch);
    }
  pr_tstamp = timestruc2hr (tmp->pr_tstamp);
  pr_create = timestruc2hr (tmp->pr_create);
  pr_term = timestruc2hr (tmp->pr_term);
  pr_rtime = timestruc2hr (tmp->pr_rtime);
  pr_utime = timestruc2hr (tmp->pr_utime);
  pr_stime = timestruc2hr (tmp->pr_stime);
  pr_ttime = timestruc2hr (tmp->pr_ttime);
  pr_tftime = timestruc2hr (tmp->pr_tftime);
  pr_dftime = timestruc2hr (tmp->pr_dftime);
  pr_kftime = timestruc2hr (tmp->pr_kftime);
  pr_ltime = timestruc2hr (tmp->pr_ltime);
  pr_slptime = timestruc2hr (tmp->pr_slptime);
  pr_wtime = timestruc2hr (tmp->pr_wtime);
  pr_stoptime = timestruc2hr (tmp->pr_stoptime);
  pr_minf = tmp->pr_minf;
  pr_majf = tmp->pr_majf;
  pr_nswap = tmp->pr_nswap;
  pr_inblk = tmp->pr_inblk;
  pr_oublk = tmp->pr_oublk;
  pr_msnd = tmp->pr_msnd;
  pr_mrcv = tmp->pr_mrcv;
  pr_sigs = tmp->pr_sigs;
  pr_vctx = tmp->pr_vctx;
  pr_ictx = tmp->pr_ictx;
  pr_sysc = tmp->pr_sysc;
  pr_ioch = tmp->pr_ioch;
  return this;
}

struct timestruc_64
{ /* 64-bit timestruc_t */
  uint64_t tv_sec;          /* seconds */
  uint64_t tv_nsec;         /* and nanoseconds */
};

typedef struct
{
  id_t pr_lwpid;            /* lwp id.  0: process or defunct */
  int pr_count;             /* number of contributing lwps */
  timestruc_64 pr_tstamp;   /* current time stamp */
  timestruc_64 pr_create;   /* process/lwp creation time stamp */
  timestruc_64 pr_term;     /* process/lwp termination time stamp */
  timestruc_64 pr_rtime;    /* total lwp real (elapsed) time */
  timestruc_64 pr_utime;    /* user level cpu time */
  timestruc_64 pr_stime;    /* system call cpu time */
  timestruc_64 pr_ttime;    /* other system trap cpu time */
  timestruc_64 pr_tftime;   /* text page fault sleep time */
  timestruc_64 pr_dftime;   /* data page fault sleep time */
  timestruc_64 pr_kftime;   /* kernel page fault sleep time */
  timestruc_64 pr_ltime;    /* user lock wait sleep time */
  timestruc_64 pr_slptime;  /* all other sleep time */
  timestruc_64 pr_wtime;    /* wait-cpu (latency) time */
  timestruc_64 pr_stoptime; /* stopped time */
  timestruc_64 filltime[6]; /* filler for future expansion */
  uint64_t pr_minf;         /* minor page faults */
  uint64_t pr_majf;         /* major page faults */
  uint64_t pr_nswap;        /* swaps */
  uint64_t pr_inblk;        /* input blocks */
  uint64_t pr_oublk;        /* output blocks */
  uint64_t pr_msnd;         /* messages sent */
  uint64_t pr_mrcv;         /* messages received */
  uint64_t pr_sigs;         /* signals received */
  uint64_t pr_vctx;         /* voluntary context switches */
  uint64_t pr_ictx;         /* involuntary context switches */
  uint64_t pr_sysc;         /* system calls */
  uint64_t pr_ioch;         /* chars read and written */
  uint64_t filler[10];      /* filler for future expansion */
} raw_prusage_64;

uint64_t
PrUsage::bind64Size ()
{
  uint64_t bindSize = sizeof (raw_prusage_64);
  return bindSize;
}

PrUsage *
PrUsage::bind64 (void *p, bool need_swap_endian)
{
  if (p == NULL)
    {
      return NULL;
    }
  raw_prusage_64 pu, *tmp = (raw_prusage_64*) p;
  if (need_swap_endian)
    {
      pu = *tmp;
      tmp = &pu;
      SWAP_ENDIAN (pu.pr_tstamp.tv_sec);
      SWAP_ENDIAN (pu.pr_tstamp.tv_nsec);
      SWAP_ENDIAN (pu.pr_create.tv_sec);
      SWAP_ENDIAN (pu.pr_create.tv_nsec);
      SWAP_ENDIAN (pu.pr_term.tv_sec);
      SWAP_ENDIAN (pu.pr_term.tv_nsec);
      SWAP_ENDIAN (pu.pr_rtime.tv_sec);
      SWAP_ENDIAN (pu.pr_rtime.tv_nsec);
      SWAP_ENDIAN (pu.pr_utime.tv_sec);
      SWAP_ENDIAN (pu.pr_utime.tv_nsec);
      SWAP_ENDIAN (pu.pr_stime.tv_sec);
      SWAP_ENDIAN (pu.pr_stime.tv_nsec);
      SWAP_ENDIAN (pu.pr_ttime.tv_sec);
      SWAP_ENDIAN (pu.pr_ttime.tv_nsec);
      SWAP_ENDIAN (pu.pr_tftime.tv_sec);
      SWAP_ENDIAN (pu.pr_tftime.tv_nsec);
      SWAP_ENDIAN (pu.pr_dftime.tv_sec);
      SWAP_ENDIAN (pu.pr_dftime.tv_nsec);
      SWAP_ENDIAN (pu.pr_kftime.tv_sec);
      SWAP_ENDIAN (pu.pr_kftime.tv_nsec);
      SWAP_ENDIAN (pu.pr_ltime.tv_sec);
      SWAP_ENDIAN (pu.pr_ltime.tv_nsec);
      SWAP_ENDIAN (pu.pr_slptime.tv_sec);
      SWAP_ENDIAN (pu.pr_slptime.tv_nsec);
      SWAP_ENDIAN (pu.pr_wtime.tv_sec);
      SWAP_ENDIAN (pu.pr_wtime.tv_nsec);
      SWAP_ENDIAN (pu.pr_stoptime.tv_sec);
      SWAP_ENDIAN (pu.pr_stoptime.tv_nsec);
      SWAP_ENDIAN (pu.pr_minf);
      SWAP_ENDIAN (pu.pr_majf);
      SWAP_ENDIAN (pu.pr_nswap);
      SWAP_ENDIAN (pu.pr_inblk);
      SWAP_ENDIAN (pu.pr_oublk);
      SWAP_ENDIAN (pu.pr_msnd);
      SWAP_ENDIAN (pu.pr_mrcv);
      SWAP_ENDIAN (pu.pr_sigs);
      SWAP_ENDIAN (pu.pr_vctx);
      SWAP_ENDIAN (pu.pr_ictx);
      SWAP_ENDIAN (pu.pr_sysc);
      SWAP_ENDIAN (pu.pr_ioch);
    }

  pr_tstamp = timestruc2hr (tmp->pr_tstamp);
  pr_create = timestruc2hr (tmp->pr_create);
  pr_term = timestruc2hr (tmp->pr_term);
  pr_rtime = timestruc2hr (tmp->pr_rtime);
  pr_utime = timestruc2hr (tmp->pr_utime);
  pr_stime = timestruc2hr (tmp->pr_stime);
  pr_ttime = timestruc2hr (tmp->pr_ttime);
  pr_tftime = timestruc2hr (tmp->pr_tftime);
  pr_dftime = timestruc2hr (tmp->pr_dftime);
  pr_kftime = timestruc2hr (tmp->pr_kftime);
  pr_ltime = timestruc2hr (tmp->pr_ltime);
  pr_slptime = timestruc2hr (tmp->pr_slptime);
  pr_wtime = timestruc2hr (tmp->pr_wtime);
  pr_stoptime = timestruc2hr (tmp->pr_stoptime);
  pr_minf = tmp->pr_minf;
  pr_majf = tmp->pr_majf;
  pr_nswap = tmp->pr_nswap;
  pr_inblk = tmp->pr_inblk;
  pr_oublk = tmp->pr_oublk;
  pr_msnd = tmp->pr_msnd;
  pr_mrcv = tmp->pr_mrcv;
  pr_sigs = tmp->pr_sigs;
  pr_vctx = tmp->pr_vctx;
  pr_ictx = tmp->pr_ictx;
  pr_sysc = tmp->pr_sysc;
  pr_ioch = tmp->pr_ioch;
  return this;
}

Vector<long long> *
PrUsage::getMstateValues ()
{
  const PrUsage *prusage = this;
  Vector<long long> *states = new Vector<long long>;
  states->store (0, prusage->pr_utime);
  states->store (1, prusage->pr_stime);
  states->store (2, prusage->pr_ttime);
  states->store (3, prusage->pr_tftime);
  states->store (4, prusage->pr_dftime);
  states->store (5, prusage->pr_kftime);
  states->store (6, prusage->pr_ltime);
  states->store (7, prusage->pr_slptime);
  states->store (8, prusage->pr_wtime);
  states->store (9, prusage->pr_stoptime);
  assert (LMS_NUM_SOLARIS_MSTATES == states->size ());
  return states;
}

void* CommonPacket::jvm_overhead = NULL;

CommonPacket::CommonPacket ()
{
  for (int i = 0; i < NTAGS; i++)
    tags[i] = 0;
  tstamp = 0;
  jthread_TBR = NULL;
  frinfo = 0;
  leafpc = 0;
  nat_stack = NULL;
  user_stack = NULL;
}

int
CommonPacket::cmp (const void *a, const void *b)
{
  if ((*(CommonPacket **) a)->tstamp > (*(CommonPacket **) b)->tstamp)
    return 1;
  else if ((*(CommonPacket **) a)->tstamp < (*(CommonPacket **) b)->tstamp)
    return -1;
  else
    return 0;
}

void *
CommonPacket::getStack (VMode view_mode)
{
  if (view_mode == VMODE_MACHINE)
    return nat_stack;
  else if (view_mode == VMODE_USER)
    {
      if (jthread_TBR == JTHREAD_NONE || (jthread_TBR && jthread_TBR->is_system ()))
	return jvm_overhead;
    }
  else if (view_mode == VMODE_EXPERT)
    {
      Histable *hist = CallStack::getStackPC (user_stack, 0);
      if (hist->get_type () == Histable::INSTR)
	{
	  DbeInstr *instr = (DbeInstr*) hist;
	  if (instr->func == dbeSession->get_JUnknown_Function ())
	    return nat_stack;
	}
      else if (hist->get_type () == Histable::LINE)
	{
	  DbeLine *line = (DbeLine *) hist;
	  if (line->func == dbeSession->get_JUnknown_Function ())
	    return nat_stack;
	}
    }
  return user_stack;
}

Histable *
CommonPacket::getStackPC (int n, VMode view_mode)
{
  return CallStack::getStackPC (getStack (view_mode), n);
}

Vector<Histable*> *
CommonPacket::getStackPCs (VMode view_mode)
{
  return CallStack::getStackPCs (getStack (view_mode));
}

void *
getStack (VMode view_mode, DataView *dview, long idx)
{
  void *stack = NULL;
  if (view_mode == VMODE_MACHINE)
    stack = dview->getObjValue (PROP_MSTACK, idx);
  else if (view_mode == VMODE_USER)
    stack = dview->getObjValue (PROP_USTACK, idx);
  else if (view_mode == VMODE_EXPERT)
    stack = dview->getObjValue (PROP_XSTACK, idx);
  return stack;
}

int
stackSize (VMode view_mode, DataView *dview, long idx)
{
  return CallStack::stackSize (getStack (view_mode, dview, idx));
}

Histable *
getStackPC (int n, VMode view_mode, DataView *dview, long idx)
{
  return CallStack::getStackPC (getStack (view_mode, dview, idx), n);
}

Vector<Histable*> *
getStackPCs (VMode view_mode, DataView *dview, long idx)
{
  return CallStack::getStackPCs (getStack (view_mode, dview, idx));
}
