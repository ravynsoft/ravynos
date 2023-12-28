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

#ifndef _HWPROFILE_H
#define _HWPROFILE_H

#include <data_pckts.h>

typedef struct Hwcntr_packet
{ /* HW counter profiling packet */
  Common_packet comm;
  uint32_t tag;         /* hw counter index, register */
  uint64_t interval;    /* overflow value */
} Hwcntr_packet;

typedef struct MHwcntr_packet
{ /* extended (superset) Hwcntr_packet */
  Common_packet comm;
  uint32_t   tag;           /* hw counter index, register */
  uint64_t   interval;      /* overflow value */
  Vaddr_type ea_vaddr;      /* virtual addr causing HWC event */
  Vaddr_type pc_vaddr;      /* candidate eventPC  */
  uint64_t   ea_paddr;      /* physical address for ea_vaddr */
  uint64_t   pc_paddr;      /* physical address for pc_vaddr */
  uint64_t   ea_pagesz;     /* pagesz (bytes) for ea_paddr */
  uint64_t   pc_pagesz;     /* pagesz (bytes) for pc_paddr */
  uint32_t   ea_lgrp;       /* latency group of ea_paddr */
  uint32_t   pc_lgrp;       /* latency group of pc_paddr */
  uint32_t   lgrp_lwp;      /* locality group of lwp */
  uint32_t   lgrp_ps;       /* locality group of process */
  uint64_t   latency;       /* latency in cycles (sampling only) */
  uint64_t   data_source;   /* data source (sampling only) */
} MHwcntr_packet;

#if ARCH(SPARC)
#define CONTEXT_PC MC_PC
#define CONTEXT_SP MC_O6
#define CONTEXT_FP MC_O7
#define SETFUNCTIONCONTEXT(ucp,funcp) \
    (ucp)->uc_mcontext.gregs[CONTEXT_PC] = (greg_t)(funcp); \
    (ucp)->uc_mcontext.gregs[CONTEXT_SP] = 0; \
    (ucp)->uc_mcontext.gregs[CONTEXT_FP] = 0;

#elif ARCH(Intel)
#include <sys/reg.h>

#if WSIZE(64)
#define CONTEXT_PC REG_RIP
#define CONTEXT_FP REG_RBP
#define CONTEXT_SP REG_RSP

#elif WSIZE(32)
#define CONTEXT_PC REG_EIP
#define CONTEXT_FP REG_EBP
#define CONTEXT_SP REG_ESP
#endif /* WSIZE() */
#define SETFUNCTIONCONTEXT(ucp,funcp) \
    (ucp)->uc_mcontext.gregs[CONTEXT_PC] = (intptr_t)(funcp); \
    (ucp)->uc_mcontext.gregs[CONTEXT_SP] = 0; \
    (ucp)->uc_mcontext.gregs[CONTEXT_FP] = 0;

#elif ARCH(Aarch64)
#define CONTEXT_PC 15
#define CONTEXT_FP 14
#define CONTEXT_SP 13
#define SETFUNCTIONCONTEXT(ucp,funcp) \
    (ucp)->uc_mcontext.regs[CONTEXT_PC] = (greg_t)(funcp); \
    (ucp)->uc_mcontext.regs[CONTEXT_SP] = 0; \
    (ucp)->uc_mcontext.regs[CONTEXT_FP] = 0;
#endif /* ARCH() */

#endif
