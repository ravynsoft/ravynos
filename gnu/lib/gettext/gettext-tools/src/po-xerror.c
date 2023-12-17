/* Error handling during reading and writing of PO files.
   Copyright (C) 2005-2007, 2013, 2019, 2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2005.

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
#include "po-xerror.h"

#include <error.h>
#include <stdio.h>
#include <stdlib.h>

#include "message.h"
#include "error-progname.h"
#include "xalloc.h"
#include "xerror.h"
#include "error.h"
#include "xvasprintf.h"
#include "po-error.h"
#if IN_LIBGETTEXTPO
# define program_name getprogname ()
#else
# include "progname.h"
#endif
#include "gettext.h"

#define _(str) gettext (str)


static void
xerror (int severity, const char *prefix_tail,
        const char *filename, size_t lineno, size_t column,
        int multiline_p, const char *message_text)
{
  if (multiline_p)
    {
      bool old_error_with_progname = error_with_progname;
      char *prefix;

      if (filename != NULL)
        {
          if (lineno != (size_t)(-1))
            {
              if (column != (size_t)(-1))
                prefix =
                  xasprintf ("%s:%ld:%ld: %s", filename,
                             (long) lineno, (long) column, prefix_tail);
              else
                prefix =
                  xasprintf ("%s:%ld: %s", filename,
                             (long) lineno, prefix_tail);
            }
          else
            prefix = xasprintf ("%s: %s", filename, prefix_tail);
          error_with_progname = false;
        }
      else
        prefix = xasprintf ("%s: %s", program_name, prefix_tail);

      if (severity >= PO_SEVERITY_ERROR)
        po_multiline_error (prefix, xstrdup (message_text));
      else
        po_multiline_warning (prefix, xstrdup (message_text));
      error_with_progname = old_error_with_progname;

      if (severity == PO_SEVERITY_FATAL_ERROR)
        exit (EXIT_FAILURE);
    }
  else
    {
      int exit_status =
        (severity == PO_SEVERITY_FATAL_ERROR ? EXIT_FAILURE : 0);

      if (filename != NULL)
        {
          error_with_progname = false;
          if (lineno != (size_t)(-1))
            {
              if (column != (size_t)(-1))
                po_error (exit_status, 0, "%s:%ld:%ld: %s%s",
                          filename, (long) lineno, (long) column,
                          prefix_tail, message_text);
              else
                po_error_at_line (exit_status, 0, filename, lineno, "%s%s",
                                  prefix_tail, message_text);
            }
          else
            po_error (exit_status, 0, "%s: %s%s",
                      filename, prefix_tail, message_text);
          error_with_progname = true;
        }
      else
        po_error (exit_status, 0, "%s%s", prefix_tail, message_text);
      if (severity < PO_SEVERITY_ERROR)
        --error_message_count;
    }
}

/* The default error handler is based on the lower-level error handler
   in po-error.h, so that gettext-po.h can offer to override one or the
   other.  */
void
textmode_xerror (int severity,
                 const struct message_ty *message,
                 const char *filename, size_t lineno, size_t column,
                 int multiline_p, const char *message_text)
{
  const char *prefix_tail =
    (severity == PO_SEVERITY_WARNING ? _("warning: ") : "");

  if (message != NULL && (filename == NULL || lineno == (size_t)(-1)))
    {
      filename = message->pos.file_name;
      lineno = message->pos.line_number;
      column = (size_t)(-1);
    }

  xerror (severity, prefix_tail, filename, lineno, column,
          multiline_p, message_text);
}

void
textmode_xerror2 (int severity,
                  const struct message_ty *message1,
                  const char *filename1, size_t lineno1, size_t column1,
                  int multiline_p1, const char *message_text1,
                  const struct message_ty *message2,
                  const char *filename2, size_t lineno2, size_t column2,
                  int multiline_p2, const char *message_text2)
{
  int severity1 = /* Don't exit before both texts have been output.  */
    (severity == PO_SEVERITY_FATAL_ERROR ? PO_SEVERITY_ERROR : severity);
  const char *prefix_tail =
    (severity == PO_SEVERITY_WARNING ? _("warning: ") : "");

  if (message1 != NULL && (filename1 == NULL || lineno1 == (size_t)(-1)))
    {
      filename1 = message1->pos.file_name;
      lineno1 = message1->pos.line_number;
      column1 = (size_t)(-1);
    }

  if (message2 != NULL && (filename2 == NULL || lineno2 == (size_t)(-1)))
    {
      filename2 = message2->pos.file_name;
      lineno2 = message2->pos.line_number;
      column2 = (size_t)(-1);
    }

  if (multiline_p1)
    xerror (severity1, prefix_tail, filename1, lineno1, column1, multiline_p1,
            message_text1);
  else
    {
      char *message_text1_extended = xasprintf ("%s...", message_text1);
      xerror (severity1, prefix_tail, filename1, lineno1, column1,
              multiline_p1, message_text1_extended);
      free (message_text1_extended);
    }

  {
    char *message_text2_extended = xasprintf ("...%s", message_text2);
    xerror (severity, prefix_tail, filename2, lineno2, column2,
            multiline_p2, message_text2_extended);
    free (message_text2_extended);
  }

  if (severity >= PO_SEVERITY_ERROR)
    /* error_message_count needs to be incremented only by 1, not by 2.  */
    --error_message_count;
}

void (*po_xerror) (int severity,
                   const struct message_ty *message,
                   const char *filename, size_t lineno, size_t column,
                   int multiline_p, const char *message_text)
  = textmode_xerror;

void (*po_xerror2) (int severity,
                    const struct message_ty *message1,
                    const char *filename1, size_t lineno1, size_t column1,
                    int multiline_p1, const char *message_text1,
                    const struct message_ty *message2,
                    const char *filename2, size_t lineno2, size_t column2,
                    int multiline_p2, const char *message_text2)
  = textmode_xerror2;
