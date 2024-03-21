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

/* Thread-specific data */

#ifndef _TSD_H
#define _TSD_H

#include <sys/types.h>

int __collector_tsd_init ();
/* Function: Init tsd module.  Call once before using other functions.
   MT-Level: Unsafe
   Return:   0 if successful
 */

void __collector_tsd_fini ();
/* Function: Shutdown tsd module.
   MT-Level: Unsafe
   Return:   None
 */

void __collector_tsd_fork_child_cleanup ();
/* Function: Reset tsd module.  Call immediately after fork() in child process.
   MT-Level: Unsafe
   Return:   None
 */

int __collector_tsd_allocate ();
/* Function: Allocate thread info.
	     Call from threads before using tsd_get_by_key().
	     Call from main thread should be made before calls from other threads.
   MT-Level: First call is unsafe.  Safe afterwards.
   Return:   0 if successful
 */

void __collector_tsd_release ();
/* Function: Free thread info.
	     Call from threads just before thread termination.
   MT-Level: Safe
   Return:   None
 */

#define COLLECTOR_TSD_INVALID_KEY ((unsigned)-1)
unsigned __collector_tsd_create_key (size_t memsize, void (*init)(void*), void (*fini)(void*));
/* Function: Reserve TDS memory.
   MT-Level: Unsafe
   Inputs:   <memsize>: number of bytes to reserve
	     <init>: key memory initialization.  Must be callable even if
		     the associated thread has not yet been created.
	     <fini>: key memory finalization.  Must be callable even if
		     the associated thread has been terminated.
   Return:   key or COLLECTOR_TSD_INVALID_KEY if not successful.
 */

void *__collector_tsd_get_by_key (unsigned key);
/* Function: Get TSD memory.
	     Call from threads after calling tsd_allocate().
   MT-Level: Safe
   Inputs:   <key>: return value from tsd_create_key()
   Return:   memory if successful, NULL otherwise
 */
#endif /* _TSD_H */
