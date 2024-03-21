/* Reading file lists.
   Copyright (C) 1995-1998, 2000-2002, 2007, 2019 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* Specification.  */
#include "file-list.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str-list.h"
#include "error.h"
#include "gettext.h"

/* A convenience macro.  I don't like writing gettext() every time.  */
#define _(str) gettext (str)


/* Read list of filenames from a file.  */
string_list_ty *
read_names_from_file (const char *file_name)
{
  size_t line_len = 0;
  char *line_buf = NULL;
  FILE *fp;
  string_list_ty *result;

  if (strcmp (file_name, "-") == 0)
    fp = stdin;
  else
    {
      fp = fopen (file_name, "r");
      if (fp == NULL)
        error (EXIT_FAILURE, errno,
               _("error while opening \"%s\" for reading"), file_name);
    }

  result = string_list_alloc ();

  while (!feof (fp))
    {
      /* Read next line from file.  */
      int len = getline (&line_buf, &line_len, fp);

      /* In case of an error leave loop.  */
      if (len < 0)
        break;

      /* Remove trailing '\n' and trailing whitespace.  */
      if (len > 0 && line_buf[len - 1] == '\n')
        line_buf[--len] = '\0';
      while (len > 0
             && (line_buf[len - 1] == ' '
                 || line_buf[len - 1] == '\t'
                 || line_buf[len - 1] == '\r'))
        line_buf[--len] = '\0';

      /* Test if we have to ignore the line.  */
      if (!(*line_buf == '\0' || *line_buf == '#'))
        /* Include the line in the result.  */
        string_list_append_unique (result, line_buf);
    }

  /* Free buffer allocated through getline.  */
  if (line_buf != NULL)
    free (line_buf);

  /* Close input stream.  */
  if (fp != stdin)
    fclose (fp);

  return result;
}
