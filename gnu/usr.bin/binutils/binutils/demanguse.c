/* demanguse.c -- libiberty demangler usage
   Copyright (C) 2021-2023 Free Software Foundation, Inc.

   This file is part of GNU Binutils.

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include <stdio.h>
#include <string.h>
#include "demangle.h"
#include "demanguse.h"

/* Print the list of demangling styles to STREAM.  A one line MSG is
   printed before the styles.  Output is limited to 80 columns, with
   continuation lines being indented by leading spaces in MSG.  */

void
display_demangler_styles (FILE *stream, const char *msg)
{
  const struct demangler_engine *info = libiberty_demanglers;
  int col;
  int lead_spaces = 0;
  const char *cont = "";

  while (msg[lead_spaces] == ' ')
    ++lead_spaces;
  col = fprintf (stream, "%s", msg);
  while (info->demangling_style_name)
    {
      if (col + strlen (info->demangling_style_name) >= 75)
	{
	  fprintf (stream, "%.1s\n", cont);
	  col = fprintf (stream, "%.*s", lead_spaces, msg);
	  cont = "";
	}
      col += fprintf (stream, "%s\"%s\"", cont, info->demangling_style_name);
      cont = ", ";
      ++info;
    }
  fprintf (stream, "\n");
}
