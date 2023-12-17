/* GNU gettext - internationalization aids
   Copyright (C) 1995-1998, 2000-2008, 2012, 2019-2020 Free Software
   Foundation, Inc.

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
# include <config.h>
#endif

/* Specification.  */
#include "write-catalog.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#ifndef STDOUT_FILENO
# define STDOUT_FILENO 1
#endif

#include <textstyle.h>

#include "fwriteerror.h"
#include "error-progname.h"
#include "xvasprintf.h"
#include "po-xerror.h"
#include "gettext.h"

/* Our regular abbreviation.  */
#define _(str) gettext (str)

/* When compiled in src, enable color support.
   When compiled in libgettextpo, don't enable color support.  */
#ifdef GETTEXTDATADIR

# define ENABLE_COLOR 1

# include "relocatable.h"
# include "po-charset.h"
# include "msgl-iconv.h"

# define GETTEXTSTYLESDIR  GETTEXTDATADIR "/styles"

#endif


/* =========== Some parameters for use by 'msgdomain_list_print'. ========== */


/* This variable controls the page width when printing messages.
   Defaults to PAGE_WIDTH if not set.  Zero (0) given to message_page_-
   width_set will result in no wrapping being performed.  */
static size_t page_width = PAGE_WIDTH;

void
message_page_width_set (size_t n)
{
  if (n == 0)
    {
      page_width = INT_MAX;
      return;
    }

  if (n < 20)
    n = 20;

  page_width = n;
}


/* ======================== msgdomain_list_print() ======================== */


void
msgdomain_list_print (msgdomain_list_ty *mdlp, const char *filename,
                      catalog_output_format_ty output_syntax,
                      bool force, bool debug)
{
  bool to_stdout;

  /* We will not write anything if, for every domain, we have no message
     or only the header entry.  */
  if (!force)
    {
      bool found_nonempty = false;
      size_t k;

      for (k = 0; k < mdlp->nitems; k++)
        {
          message_list_ty *mlp = mdlp->item[k]->messages;

          if (!(mlp->nitems == 0
                || (mlp->nitems == 1 && is_header (mlp->item[0]))))
            {
              found_nonempty = true;
              break;
            }
        }

      if (!found_nonempty)
        return;
    }

  /* Check whether the output format can accommodate all messages.  */
  if (!output_syntax->supports_multiple_domains && mdlp->nitems > 1)
    {
      if (output_syntax->alternative_is_po)
        po_xerror (PO_SEVERITY_FATAL_ERROR, NULL, NULL, 0, 0, false,
                   _("Cannot output multiple translation domains into a single file with the specified output format. Try using PO file syntax instead."));
      else
        po_xerror (PO_SEVERITY_FATAL_ERROR, NULL, NULL, 0, 0, false,
                   _("Cannot output multiple translation domains into a single file with the specified output format."));
    }
  else
    {
      if (!output_syntax->supports_contexts)
        {
          const lex_pos_ty *has_context;
          size_t k;

          has_context = NULL;
          for (k = 0; k < mdlp->nitems; k++)
            {
              message_list_ty *mlp = mdlp->item[k]->messages;
              size_t j;

              for (j = 0; j < mlp->nitems; j++)
                {
                  message_ty *mp = mlp->item[j];

                  if (mp->msgctxt != NULL)
                    {
                      has_context = &mp->pos;
                      break;
                    }
                }
            }

          if (has_context != NULL)
            {
              error_with_progname = false;
              po_xerror (PO_SEVERITY_FATAL_ERROR, NULL,
                         has_context->file_name, has_context->line_number,
                         (size_t)(-1), false,
                         _("message catalog has context dependent translations, but the output format does not support them."));
              error_with_progname = true;
            }
        }

      if (!output_syntax->supports_plurals)
        {
          const lex_pos_ty *has_plural;
          size_t k;

          has_plural = NULL;
          for (k = 0; k < mdlp->nitems; k++)
            {
              message_list_ty *mlp = mdlp->item[k]->messages;
              size_t j;

              for (j = 0; j < mlp->nitems; j++)
                {
                  message_ty *mp = mlp->item[j];

                  if (mp->msgid_plural != NULL)
                    {
                      has_plural = &mp->pos;
                      break;
                    }
                }
            }

          if (has_plural != NULL)
            {
              error_with_progname = false;
              if (output_syntax->alternative_is_java_class)
                po_xerror (PO_SEVERITY_FATAL_ERROR, NULL,
                           has_plural->file_name, has_plural->line_number,
                           (size_t)(-1), false,
                           _("message catalog has plural form translations, but the output format does not support them. Try generating a Java class using \"msgfmt --java\", instead of a properties file."));
              else
                po_xerror (PO_SEVERITY_FATAL_ERROR, NULL,
                           has_plural->file_name, has_plural->line_number,
                           (size_t)(-1), false,
                           _("message catalog has plural form translations, but the output format does not support them."));
              error_with_progname = true;
            }
        }
    }

  to_stdout = (filename == NULL || strcmp (filename, "-") == 0
               || strcmp (filename, "/dev/stdout") == 0);

#if ENABLE_COLOR
  if (output_syntax->supports_color
      && (color_mode == color_yes
          || (color_mode == color_tty && to_stdout
              && isatty (STDOUT_FILENO)
              && getenv ("NO_COLOR") == NULL)))
    {
      int fd;
      ostream_t stream;

      /* Open the output file.  */
      if (!to_stdout)
        {
          fd = open (filename, O_WRONLY | O_CREAT | O_TRUNC,
                     /* 0666 in portable POSIX notation: */
                     S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
          if (fd < 0)
            {
              const char *errno_description = strerror (errno);
              po_xerror (PO_SEVERITY_FATAL_ERROR, NULL, NULL, 0, 0, false,
                         xasprintf ("%s: %s",
                                    xasprintf (_("cannot create output file \"%s\""),
                                               filename),
                                    errno_description));
            }
        }
      else
        {
          fd = STDOUT_FILENO;
          filename = _("standard output");
        }

      style_file_prepare ("PO_STYLE",
                          "GETTEXTSTYLESDIR", relocate (GETTEXTSTYLESDIR),
                          "po-default.css");
      stream =
        styled_ostream_create (fd, filename, TTYCTL_AUTO, style_file_name);
      output_syntax->print (mdlp, stream, page_width, debug);
      ostream_free (stream);

      /* Make sure nothing went wrong.  */
      if (close (fd) < 0)
        {
          const char *errno_description = strerror (errno);
          po_xerror (PO_SEVERITY_FATAL_ERROR, NULL, NULL, 0, 0, false,
                     xasprintf ("%s: %s",
                                xasprintf (_("error while writing \"%s\" file"),
                                           filename),
                                errno_description));
        }
    }
  else
#endif
    {
      FILE *fp;
      file_ostream_t stream;

      /* Open the output file.  */
      if (!to_stdout)
        {
          fp = fopen (filename, "wb");
          if (fp == NULL)
            {
              const char *errno_description = strerror (errno);
              po_xerror (PO_SEVERITY_FATAL_ERROR, NULL, NULL, 0, 0, false,
                         xasprintf ("%s: %s",
                                    xasprintf (_("cannot create output file \"%s\""),
                                               filename),
                                    errno_description));
            }
        }
      else
        {
          fp = stdout;
          filename = _("standard output");
        }

      stream = file_ostream_create (fp);

#if ENABLE_COLOR
      if (output_syntax->supports_color && color_mode == color_html)
        {
          html_styled_ostream_t html_stream;

          /* Convert mdlp to UTF-8 encoding.  */
          if (mdlp->encoding != po_charset_utf8)
            {
              mdlp = msgdomain_list_copy (mdlp, 0);
              mdlp = iconv_msgdomain_list (mdlp, po_charset_utf8, false, NULL);
            }

          style_file_prepare ("PO_STYLE",
                              "GETTEXTSTYLESDIR", relocate (GETTEXTSTYLESDIR),
                              "po-default.css");
          html_stream = html_styled_ostream_create (stream, style_file_name);
          output_syntax->print (mdlp, html_stream, page_width, debug);
          ostream_free (html_stream);
        }
      else
        {
          noop_styled_ostream_t styled_stream;

          styled_stream = noop_styled_ostream_create (stream, false);
          output_syntax->print (mdlp, styled_stream, page_width, debug);
          ostream_free (styled_stream);
        }
#else
      output_syntax->print (mdlp, stream, page_width, debug);
      /* Don't call ostream_free if file_ostream_create is a dummy.  */
      if (stream != fp)
#endif
        ostream_free (stream);

      /* Make sure nothing went wrong.  */
      if (fwriteerror (fp))
        {
          const char *errno_description = strerror (errno);
          po_xerror (PO_SEVERITY_FATAL_ERROR, NULL, NULL, 0, 0, false,
                     xasprintf ("%s: %s",
                                xasprintf (_("error while writing \"%s\" file"),
                                           filename),
                                errno_description));
        }
    }
}


/* =============================== Sorting. ================================ */


static int
cmp_by_msgid (const void *va, const void *vb)
{
  const message_ty *a = *(const message_ty **) va;
  const message_ty *b = *(const message_ty **) vb;

  /* Because msgids normally contain only ASCII characters or are UTF-8
     encoded, it is OK to sort them as if we were in a C.UTF-8 locale. And
     strcoll() in a C.UTF-8 locale is the same as strcmp().  */
  int cmp = strcmp (a->msgid, b->msgid);
  if (cmp != 0)
    return cmp;

  /* If the msgids are equal, disambiguate by comparing the contexts.  */
  if (a->msgctxt == b->msgctxt)
    return 0;
  if (a->msgctxt == NULL)
    return -1;
  if (b->msgctxt == NULL)
    return 1;
  return strcmp (a->msgctxt, b->msgctxt);
}


void
msgdomain_list_sort_by_msgid (msgdomain_list_ty *mdlp)
{
  size_t k;

  for (k = 0; k < mdlp->nitems; k++)
    {
      message_list_ty *mlp = mdlp->item[k]->messages;

      if (mlp->nitems > 0)
        qsort (mlp->item, mlp->nitems, sizeof (mlp->item[0]), cmp_by_msgid);
    }
}


/* Sort the file positions of every message.  */

static int
cmp_filepos (const void *va, const void *vb)
{
  const lex_pos_ty *a = (const lex_pos_ty *) va;
  const lex_pos_ty *b = (const lex_pos_ty *) vb;
  int cmp;

  cmp = strcmp (a->file_name, b->file_name);
  if (cmp == 0)
    cmp = (int) a->line_number - (int) b->line_number;

  return cmp;
}

static void
msgdomain_list_sort_filepos (msgdomain_list_ty *mdlp)
{
  size_t j, k;

  for (k = 0; k < mdlp->nitems; k++)
    {
      message_list_ty *mlp = mdlp->item[k]->messages;

      for (j = 0; j < mlp->nitems; j++)
        {
          message_ty *mp = mlp->item[j];

          if (mp->filepos_count > 0)
            qsort (mp->filepos, mp->filepos_count, sizeof (mp->filepos[0]),
                   cmp_filepos);
        }
    }
}


/* Sort the messages according to the file position.  */

static int
cmp_by_filepos (const void *va, const void *vb)
{
  const message_ty *a = *(const message_ty **) va;
  const message_ty *b = *(const message_ty **) vb;
  int cmp;

  /* No filepos is smaller than any other filepos.  */
  cmp = (a->filepos_count != 0) - (b->filepos_count != 0);
  if (cmp != 0)
    return cmp;

  if (a->filepos_count != 0)
    {
      /* Compare on the file names...  */
      cmp = strcmp (a->filepos[0].file_name, b->filepos[0].file_name);
      if (cmp != 0)
        return cmp;

      /* If they are equal, compare on the line numbers...  */
      cmp = a->filepos[0].line_number - b->filepos[0].line_number;
      if (cmp != 0)
        return cmp;
    }

  /* If they are equal, compare on the msgid strings.  */
  /* Because msgids normally contain only ASCII characters or are UTF-8
     encoded, it is OK to sort them as if we were in a C.UTF-8 locale. And
     strcoll() in a C.UTF-8 locale is the same as strcmp().  */
  cmp = strcmp (a->msgid, b->msgid);
  if (cmp != 0)
    return cmp;

  /* If the msgids are equal, disambiguate by comparing the contexts.  */
  if (a->msgctxt == b->msgctxt)
    return 0;
  if (a->msgctxt == NULL)
    return -1;
  if (b->msgctxt == NULL)
    return 1;
  return strcmp (a->msgctxt, b->msgctxt);
}


void
msgdomain_list_sort_by_filepos (msgdomain_list_ty *mdlp)
{
  size_t k;

  /* It makes sense to compare filepos[0] of different messages only after
     the filepos[] array of each message has been sorted.  Sort it now.  */
  msgdomain_list_sort_filepos (mdlp);

  for (k = 0; k < mdlp->nitems; k++)
    {
      message_list_ty *mlp = mdlp->item[k]->messages;

      if (mlp->nitems > 0)
        qsort (mlp->item, mlp->nitems, sizeof (mlp->item[0]), cmp_by_filepos);
    }
}
