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
#include <new>          // std::bad_alloc
#include <stdio.h>      // fprintf
#include <stdlib.h>     // exit
#include "DbeApplication.h"

static char *name = NULL;

/**
 * Out Of Memory exception handler
 */
void
out_of_mem ()
{
  fprintf (stderr, "%s: %s: %s\n", "Error", name ? name : "", "Out of memory\n");
  exit (2); // Out of memory
  // throw bad_alloc();
}

/**
 * Calls real_main inside try{...}catch(std::bad_alloc *)
 */
int
catch_out_of_memory (int (*real_main)(int, char*[]), int argc, char *argv[])
{
  int i = 0;
  name = argv[0];
  std::set_new_handler (out_of_mem);
  try
    {
      i = real_main (argc, argv);
    }
  catch (std::bad_alloc */*ba*/)
    {
      exit (2); // Out of memory
    }
  delete theDbeApplication;
  return i;
}
