/* Copyright (C) 2007-2023 Free Software Foundation, Inc.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

/* Intel 386 running any BSD Unix.  */

#include <machine/param.h>
#include <machine/vmparam.h>

/* Recent versions of FreeBSD don't define NBPG.  */
#ifndef NBPG
#ifdef PAGE_SIZE
#define NBPG PAGE_SIZE
#endif
#endif

#define	HOST_PAGE_SIZE		NBPG
#define	HOST_MACHINE_ARCH	bfd_arch_i386
#define	HOST_TEXT_START_ADDR		USRTEXT

/* Jolitz suggested defining HOST_STACK_END_ADDR to
   (u.u_kproc.kp_eproc.e_vm.vm_maxsaddr + MAXSSIZ), which should work on
   both BSDI and 386BSD, but that is believed not to work for BSD 4.4.  */

#ifdef __bsdi__
/* This seems to be the right thing for BSDI.  */
#define	HOST_STACK_END_ADDR		USRSTACK
#define HOST_DATA_START_ADDR ((bfd_vma)u.u_kproc.kp_eproc.e_vm.vm_daddr)
#else
/* This seems to be the right thing for 386BSD release 0.1.  */
#define	HOST_STACK_END_ADDR		(USRSTACK - MAXSSIZ)
#endif

#define TRAD_UNIX_CORE_FILE_FAILING_SIGNAL(core_bfd) \
  ((core_bfd)->tdata.trad_core_data->u.u_sig)
#define u_comm u_kproc.kp_proc.p_comm
