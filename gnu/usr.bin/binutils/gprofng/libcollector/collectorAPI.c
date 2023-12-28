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

/* C and Fortran stubs for collector API */

#include "config.h"
#include <dlfcn.h>
#include "gp-defs.h"
#include "collectorAPI.h"
#include "gp-experiment.h"

static void *__real_collector_sample = NULL;
static void *__real_collector_pause = NULL;
static void *__real_collector_resume = NULL;
static void *__real_collector_terminate_expt = NULL;
static void *__real_collector_func_load = NULL;
static void *__real_collector_func_unload = NULL;

#define INIT_API        if (init_API == 0) collectorAPI_initAPI()
#define NULL_PTR(x)     (__real_##x == NULL)
#define CALL_REAL(x)    (*(void(*)())__real_##x)
#define CALL_IF_REAL(x) INIT_API; if (!NULL_PTR(x)) CALL_REAL(x)

static int init_API = 0;

void
collectorAPI_initAPI (void)
{
  void *libcollector = dlopen (SP_LIBCOLLECTOR_NAME, RTLD_NOLOAD);
  if (libcollector == NULL)
    libcollector = RTLD_DEFAULT;
  __real_collector_sample = dlsym (libcollector, "__collector_sample");
  __real_collector_pause = dlsym (libcollector, "__collector_pause");
  __real_collector_resume = dlsym (libcollector, "__collector_resume");
  __real_collector_terminate_expt = dlsym (libcollector, "__collector_terminate_expt");
  __real_collector_func_load = dlsym (libcollector, "__collector_func_load");
  __real_collector_func_unload = dlsym (libcollector, "__collector_func_unload");
  init_API = 1;
}

/* initialization -- init section routine */
static void collectorAPI_init () __attribute__ ((constructor));

static void
collectorAPI_init (void)
{
  collectorAPI_initAPI ();
}

/* C API */
void
collector_pause (void)
{
  CALL_IF_REAL (collector_pause)();
}

void
collector_resume (void)
{
  CALL_IF_REAL (collector_resume)();
}

void
collector_sample (const char *name)
{
  CALL_IF_REAL (collector_sample)(name);
}

void
collector_terminate_expt (void)
{
  CALL_IF_REAL (collector_terminate_expt)();
}

void
collector_func_load (const char *name, const char *alias, const char *sourcename,
		     void *vaddr, int size, int lntsize, Lineno *lntable)
{
  CALL_IF_REAL (collector_func_load)(name, alias, sourcename,
				     vaddr, size, lntsize, lntable);
}

void
collector_func_unload (void *vaddr)
{
  CALL_IF_REAL (collector_func_unload)(vaddr);
}

/* Fortran API */
void
collector_pause_ (void)
{
  CALL_IF_REAL (collector_pause)();
}

void
collector_resume_ (void)
{
  CALL_IF_REAL (collector_resume)();
}

void
collector_terminate_expt_ (void)
{
  CALL_IF_REAL (collector_terminate_expt)();
}

void
collector_sample_ (char *name, long name_length)
{
  INIT_API;
  if (!NULL_PTR (collector_sample))
    {
      char name_string[256];
      long length = sizeof (name_string) - 1;
      if (name_length < length)
	length = name_length;
      for (long i = 0; i < length; i++)
	name_string[i] = name[i];
      name_string[length] = '\0';
      CALL_REAL (collector_sample)(name_string);
    }
}
