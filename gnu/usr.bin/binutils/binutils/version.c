/* version.c -- binutils version information
   Copyright (C) 1991-2023 Free Software Foundation, Inc.

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

#include "sysdep.h"
#include "bfd.h"
#include "bucomm.h"

/* Print the version number and copyright information, and exit.
   This implements the --version option for the various programs.  */

void
print_version (const char *name)
{
  /* This output is intended to follow the GNU standards document.  */
  /* xgettext:c-format */
  printf ("GNU %s %s\n", name, BFD_VERSION_STRING);
  printf (_("Copyright (C) 2023 Free Software Foundation, Inc.\n"));
  printf (_("\
This program is free software; you may redistribute it under the terms of\n\
the GNU General Public License version 3 or (at your option) any later version.\n\
This program has absolutely no warranty.\n"));
  exit (0);
}
