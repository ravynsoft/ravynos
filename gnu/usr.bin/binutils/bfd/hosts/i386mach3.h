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

#include <machine/vmparam.h>
#include <sys/param.h>

/* This is an ugly way to hack around the incorrect
 * definition of UPAGES in i386/machparam.h.
 *
 * The definition should specify the size reserved
 * for "struct user" in core files in PAGES,
 * but instead it gives it in 512-byte core-clicks
 * for i386 and i860. UPAGES is used only in trad-core.c.
 */
#if UPAGES == 16
#undef  UPAGES
#define UPAGES 2
#endif

#if UPAGES != 2
FIXME!! UPAGES is neither 2 nor 16
#endif

#define	HOST_PAGE_SIZE		1
#define	HOST_SEGMENT_SIZE	NBPG
#define	HOST_MACHINE_ARCH	bfd_arch_i386
#define	HOST_TEXT_START_ADDR	USRTEXT
#define	HOST_STACK_END_ADDR	USRSTACK
