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

/* Lineage events for process fork, exec, etc. */

#ifndef DESCENDANTS_H
#define DESCENDANTS_H

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <alloca.h>
#include <assert.h>

#include "gp-defs.h"
#include "gp-experiment.h"
#include "collector.h"
#include "memmgr.h"
#include "cc_libcollector.h"
#include "tsd.h"

/* configuration, not changed after init. */
typedef enum
{
  LM_DORMANT        = -2,   /* env vars preserved, not recording */
  LM_CLOSED         = -1,   /* env vars cleared, not recording */
  LM_TRACK_LINEAGE  = 1,    /* env vars preserved, recording */
} line_mode_t;

extern line_mode_t line_mode;
extern int user_follow_mode;
extern int java_mode;
extern int dbg_current_mode; 	/* for debug only */
extern unsigned line_key;
extern char **sp_env_backup;

#define INIT_REENTRANCE(x)  ((x) = __collector_tsd_get_by_key (line_key))
#define CHCK_REENTRANCE(x)  (((INIT_REENTRANCE(x)) == NULL) || (*(x) != 0))
#define PUSH_REENTRANCE(x)  ((*(x))++)
#define POP_REENTRANCE(x)   ((*(x))--)

/* environment variables that must be forwarded to descendents */
#define SP_COLLECTOR_PARAMS         "SP_COLLECTOR_PARAMS"
#define SP_COLLECTOR_EXPNAME        "SP_COLLECTOR_EXPNAME"
#define SP_COLLECTOR_FOLLOW_SPEC    "SP_COLLECTOR_FOLLOW_SPEC"
#define SP_COLLECTOR_FOUNDER        "SP_COLLECTOR_FOUNDER"
#define SP_PRELOAD_STRINGS          "SP_COLLECTOR_PRELOAD"
#define LD_PRELOAD_STRINGS          "LD_PRELOAD"
#define SP_LIBPATH_STRINGS          "SP_COLLECTOR_LIBRARY_PATH"
#define LD_LIBPATH_STRINGS          "LD_LIBRARY_PATH"
#define JAVA_TOOL_OPTIONS          "JAVA_TOOL_OPTIONS"
#define COLLECTOR_JVMTI_OPTION     "-agentlib:gp-collector"

extern int __collector_linetrace_shutdown_hwcs_6830763_XXXX;
extern void __collector_env_unset (char *envp[]);
extern void __collector_env_save_preloads ();
extern char ** __collector_env_backup ();
extern void __collector_env_backup_free ();
extern void __collector_env_update (char *envp[]);
extern void __collector_env_print (char *label);
extern void __collector_env_printall (char *label, char *envp[]);
extern char ** __collector_env_allocate (char *const old_env[], int allocate_env);

#endif
