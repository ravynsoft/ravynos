/* Reading PO files, abstract class.
   Copyright (C) 1995-1996, 1998, 2000-2009, 2013, 2015, 2021, 2023 Free Software Foundation, Inc.

   This file was written by Peter Miller <millerp@canb.auug.org.au>

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
#include "read-catalog-abstract.h"

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "po-charset.h"
#include "xalloc.h"
#include "xvasprintf.h"
#include "po-xerror.h"
#include "error.h"
#include "gettext.h"

/* Local variables.  */
static abstract_catalog_reader_ty *callback_arg;


/* ========================================================================= */
/* Allocating and freeing instances of abstract_catalog_reader_ty.  */


abstract_catalog_reader_ty *
catalog_reader_alloc (abstract_catalog_reader_class_ty *method_table)
{
  abstract_catalog_reader_ty *pop;

  pop = (abstract_catalog_reader_ty *) xmalloc (method_table->size);
  pop->methods = method_table;
  if (method_table->constructor)
    method_table->constructor (pop);
  return pop;
}


void
catalog_reader_free (abstract_catalog_reader_ty *pop)
{
  if (pop->methods->destructor)
    pop->methods->destructor (pop);
  free (pop);
}


/* ========================================================================= */
/* Inline functions to invoke the methods.  */


static inline void
call_parse_brief (abstract_catalog_reader_ty *pop)
{
  if (pop->methods->parse_brief)
    pop->methods->parse_brief (pop);
}

static inline void
call_parse_debrief (abstract_catalog_reader_ty *pop)
{
  if (pop->methods->parse_debrief)
    pop->methods->parse_debrief (pop);
}

static inline void
call_directive_domain (abstract_catalog_reader_ty *pop, char *name)
{
  if (pop->methods->directive_domain)
    pop->methods->directive_domain (pop, name);
}

static inline void
call_directive_message (abstract_catalog_reader_ty *pop,
                        char *msgctxt,
                        char *msgid,
                        lex_pos_ty *msgid_pos,
                        char *msgid_plural,
                        char *msgstr, size_t msgstr_len,
                        lex_pos_ty *msgstr_pos,
                        char *prev_msgctxt,
                        char *prev_msgid,
                        char *prev_msgid_plural,
                        bool force_fuzzy, bool obsolete)
{
  if (pop->methods->directive_message)
    pop->methods->directive_message (pop, msgctxt,
                                     msgid, msgid_pos, msgid_plural,
                                     msgstr, msgstr_len, msgstr_pos,
                                     prev_msgctxt,
                                     prev_msgid,
                                     prev_msgid_plural,
                                     force_fuzzy, obsolete);
}

static inline void
call_comment (abstract_catalog_reader_ty *pop, const char *s)
{
  if (pop->methods->comment != NULL)
    pop->methods->comment (pop, s);
}

static inline void
call_comment_dot (abstract_catalog_reader_ty *pop, const char *s)
{
  if (pop->methods->comment_dot != NULL)
    pop->methods->comment_dot (pop, s);
}

static inline void
call_comment_filepos (abstract_catalog_reader_ty *pop, const char *name,
                      size_t line)
{
  if (pop->methods->comment_filepos)
    pop->methods->comment_filepos (pop, name, line);
}

static inline void
call_comment_special (abstract_catalog_reader_ty *pop, const char *s)
{
  if (pop->methods->comment_special != NULL)
    pop->methods->comment_special (pop, s);
}


/* ========================================================================= */
/* Exported functions.  */


static inline void
parse_start (abstract_catalog_reader_ty *pop)
{
  /* The parse will call the po_callback_... functions (see below)
     when the various directive are recognised.  The callback_arg
     variable is used to tell these functions which instance is to
     have the relevant method invoked.  */
  callback_arg = pop;

  call_parse_brief (pop);
}

static inline void
parse_end (abstract_catalog_reader_ty *pop)
{
  call_parse_debrief (pop);
  callback_arg = NULL;
}


void
catalog_reader_parse (abstract_catalog_reader_ty *pop, FILE *fp,
                      const char *real_filename, const char *logical_filename,
                      catalog_input_format_ty input_syntax)
{
  error_message_count = 0;

  /* Parse the stream's content.  */
  parse_start (pop);
  input_syntax->parse (pop, fp, real_filename, logical_filename);
  parse_end (pop);

  if (error_message_count > 0)
    po_xerror (PO_SEVERITY_FATAL_ERROR, NULL,
               /*real_filename*/ NULL, (size_t)(-1), (size_t)(-1), false,
               xasprintf (ngettext ("found %u fatal error",
                                    "found %u fatal errors",
                                    error_message_count),
                          error_message_count));
}


/* ========================================================================= */
/* Callbacks used by po-gram.y or po-lex.c, indirectly from
   catalog_reader_parse.  */


/* This function is called by po_gram_lex() whenever a domain directive
   has been seen.  */
void
po_callback_domain (char *name)
{
  /* assert(callback_arg); */
  call_directive_domain (callback_arg, name);
}


/* This function is called by po_gram_lex() whenever a message has been
   seen.  */
void
po_callback_message (char *msgctxt,
                     char *msgid, lex_pos_ty *msgid_pos, char *msgid_plural,
                     char *msgstr, size_t msgstr_len, lex_pos_ty *msgstr_pos,
                     char *prev_msgctxt,
                     char *prev_msgid,
                     char *prev_msgid_plural,
                     bool force_fuzzy, bool obsolete)
{
  /* assert(callback_arg); */
  call_directive_message (callback_arg, msgctxt,
                          msgid, msgid_pos, msgid_plural,
                          msgstr, msgstr_len, msgstr_pos,
                          prev_msgctxt, prev_msgid, prev_msgid_plural,
                          force_fuzzy, obsolete);
}


void
po_callback_comment (const char *s)
{
  /* assert(callback_arg); */
  call_comment (callback_arg, s);
}


void
po_callback_comment_dot (const char *s)
{
  /* assert(callback_arg); */
  call_comment_dot (callback_arg, s);
}


/* This function is called by po_parse_comment_filepos(), once for each
   filename.  */
void
po_callback_comment_filepos (const char *name, size_t line)
{
  /* assert(callback_arg); */
  call_comment_filepos (callback_arg, name, line);
}


void
po_callback_comment_special (const char *s)
{
  /* assert(callback_arg); */
  call_comment_special (callback_arg, s);
}


/* Parse a special comment and put the result in *fuzzyp, formatp, *rangep,
   *wrapp.  */
void
po_parse_comment_special (const char *s,
                          bool *fuzzyp, enum is_format formatp[NFORMATS],
                          struct argument_range *rangep, enum is_wrap *wrapp,
                          enum is_syntax_check scp[NSYNTAXCHECKS])
{
  size_t i;

  *fuzzyp = false;
  for (i = 0; i < NFORMATS; i++)
    formatp[i] = undecided;
  rangep->min = -1;
  rangep->max = -1;
  *wrapp = undecided;
  for (i = 0; i < NSYNTAXCHECKS; i++)
    scp[i] = undecided;

  while (*s != '\0')
    {
      const char *t;

      /* Skip whitespace.  */
      while (*s != '\0' && strchr ("\n \t\r\f\v,", *s) != NULL)
        s++;

      /* Collect a token.  */
      t = s;
      while (*s != '\0' && strchr ("\n \t\r\f\v,", *s) == NULL)
        s++;
      if (s != t)
        {
          size_t len = s - t;

          /* Accept fuzzy flag.  */
          if (len == 5 && memcmp (t, "fuzzy", 5) == 0)
            {
              *fuzzyp = true;
              continue;
            }

          /* Accept format description.  */
          if (len >= 7 && memcmp (t + len - 7, "-format", 7) == 0)
            {
              const char *p;
              size_t n;
              enum is_format value;

              p = t;
              n = len - 7;

              if (n >= 3 && memcmp (p, "no-", 3) == 0)
                {
                  p += 3;
                  n -= 3;
                  value = no;
                }
              else if (n >= 9 && memcmp (p, "possible-", 9) == 0)
                {
                  p += 9;
                  n -= 9;
                  value = possible;
                }
              else if (n >= 11 && memcmp (p, "impossible-", 11) == 0)
                {
                  p += 11;
                  n -= 11;
                  value = impossible;
                }
              else
                value = yes;

              for (i = 0; i < NFORMATS; i++)
                if (strlen (format_language[i]) == n
                    && memcmp (format_language[i], p, n) == 0)
                  {
                    formatp[i] = value;
                    break;
                  }
              if (i < NFORMATS)
                continue;
            }

          /* Accept range description "range: <min>..<max>".  */
          if (len == 6 && memcmp (t, "range:", 6) == 0)
            {
              /* Skip whitespace.  */
              while (*s != '\0' && strchr ("\n \t\r\f\v,", *s) != NULL)
                s++;

              /* Collect a token.  */
              t = s;
              while (*s != '\0' && strchr ("\n \t\r\f\v,", *s) == NULL)
                s++;
              /* Parse it.  */
              if (*t >= '0' && *t <= '9')
                {
                  unsigned int min = 0;

                  for (; *t >= '0' && *t <= '9'; t++)
                    {
                      if (min <= INT_MAX / 10)
                        {
                          min = 10 * min + (*t - '0');
                          if (min > INT_MAX)
                            min = INT_MAX;
                        }
                      else
                        /* Avoid integer overflow.  */
                        min = INT_MAX;
                    }
                  if (*t++ == '.')
                    if (*t++ == '.')
                      if (*t >= '0' && *t <= '9')
                        {
                          unsigned int max = 0;
                          for (; *t >= '0' && *t <= '9'; t++)
                            {
                              if (max <= INT_MAX / 10)
                                {
                                  max = 10 * max + (*t - '0');
                                  if (max > INT_MAX)
                                    max = INT_MAX;
                                }
                              else
                                /* Avoid integer overflow.  */
                                max = INT_MAX;
                            }
                          if (min <= max)
                            {
                              rangep->min = min;
                              rangep->max = max;
                              continue;
                            }
                        }
                }
            }

          /* Accept wrap description.  */
          if (len == 4 && memcmp (t, "wrap", 4) == 0)
            {
              *wrapp = yes;
              continue;
            }
          if (len == 7 && memcmp (t, "no-wrap", 7) == 0)
            {
              *wrapp = no;
              continue;
            }

          /* Accept syntax check description.  */
          if (len >= 6 && memcmp (t + len - 6, "-check", 6) == 0)
            {
              const char *p;
              size_t n;
              enum is_syntax_check value;

              p = t;
              n = len - 6;

              if (n >= 3 && memcmp (p, "no-", 3) == 0)
                {
                  p += 3;
                  n -= 3;
                  value = no;
                }
              else
                value = yes;

              for (i = 0; i < NSYNTAXCHECKS; i++)
                if (strlen (syntax_check_name[i]) == n
                    && memcmp (syntax_check_name[i], p, n) == 0)
                  {
                    scp[i] = value;
                    break;
                  }
              if (i < NSYNTAXCHECKS)
                continue;
            }

          /* Unknown special comment marker.  It may have been generated
             from a future xgettext version.  Ignore it.  */
        }
    }
}


/* Parse a GNU style file comment.
   Syntax: an arbitrary number of
             STRING COLON NUMBER
           or
             STRING
   The latter style, without line number, occurs in PO files converted e.g.
   from Pascal .rst files or from OpenOffice resource files.
   The STRING is either
             FILENAME
           or
             U+2068 FILENAME U+2069.
   Call po_callback_comment_filepos for each of them.  */
static void
po_parse_comment_filepos (const char *s)
{
  while (*s != '\0')
    {
      while (*s == ' ' || *s == '\t' || *s == '\n')
        s++;
      if (*s != '\0')
        {
          bool isolated_filename =
            (po_lex_isolate_start != NULL
             && strncmp (s, po_lex_isolate_start,
                         strlen (po_lex_isolate_start)) == 0);
          if (isolated_filename)
            s += strlen (po_lex_isolate_start);

          const char *filename_start = s;
          const char *filename_end;

          if (isolated_filename)
            {
              for (;; s++)
                {
                  if (*s == '\0' || *s == '\n')
                    {
                      filename_end = s;
                      break;
                    }
                  if (strncmp (s, po_lex_isolate_end,
                               strlen (po_lex_isolate_end)) == 0)
                    {
                      filename_end = s;
                      s += strlen (po_lex_isolate_end);
                      break;
                    }
                }
            }
          else
            {
              do
                s++;
              while (!(*s == '\0' || *s == ' ' || *s == '\t' || *s == '\n'));
              filename_end = s;
            }

          /* See if there is a COLON and NUMBER after the STRING, separated
             through optional spaces.  */
          {
            const char *p = s;

            while (*p == ' ' || *p == '\t' || *p == '\n')
              p++;

            if (*p == ':')
              {
                p++;

                while (*p == ' ' || *p == '\t' || *p == '\n')
                  p++;

                if (*p >= '0' && *p <= '9')
                  {
                    /* Accumulate a number.  */
                    size_t n = 0;

                    do
                      {
                        n = n * 10 + (*p - '0');
                        p++;
                      }
                    while (*p >= '0' && *p <= '9');

                    if (*p == '\0' || *p == ' ' || *p == '\t' || *p == '\n')
                      {
                        /* Parsed a GNU style file comment with spaces.  */
                        size_t filename_length = filename_end - filename_start;
                        char *filename = XNMALLOC (filename_length + 1, char);

                        memcpy (filename, filename_start, filename_length);
                        filename[filename_length] = '\0';

                        po_callback_comment_filepos (filename, n);

                        free (filename);

                        s = p;
                        continue;
                      }
                  }
              }
          }

          /* See if there is a COLON at the end of STRING and a NUMBER after
             it, separated through optional spaces.  */
          if (s[-1] == ':')
            {
              const char *p = s;

              while (*p == ' ' || *p == '\t' || *p == '\n')
                p++;

              if (*p >= '0' && *p <= '9')
                {
                  /* Accumulate a number.  */
                  size_t n = 0;

                  do
                    {
                      n = n * 10 + (*p - '0');
                      p++;
                    }
                  while (*p >= '0' && *p <= '9');

                  if (*p == '\0' || *p == ' ' || *p == '\t' || *p == '\n')
                    {
                      /* Parsed a GNU style file comment with spaces.  */
                      filename_end = s - 1;
                      size_t filename_length = filename_end - filename_start;
                      char *filename = XNMALLOC (filename_length + 1, char);

                      memcpy (filename, filename_start, filename_length);
                      filename[filename_length] = '\0';

                      po_callback_comment_filepos (filename, n);

                      free (filename);

                      s = p;
                      continue;
                    }
                }
            }

          /* See if there is a COLON and NUMBER at the end of the STRING,
             without separating spaces.  */
          {
            const char *p = s;

            while (p > filename_start)
              {
                p--;
                if (!(*p >= '0' && *p <= '9'))
                  {
                    p++;
                    break;
                  }
              }

            /* p now points to the beginning of the trailing digits segment
               at the end of STRING.  */

            if (p < s
                && p > filename_start + 1
                && p[-1] == ':')
              {
                /* Parsed a GNU style file comment without spaces.  */
                const char *string_end = p - 1;

                /* Accumulate a number.  */
                {
                  size_t n = 0;

                  do
                    {
                      n = n * 10 + (*p - '0');
                      p++;
                    }
                  while (p < s);

                  {
                    filename_end = string_end;
                    size_t filename_length = filename_end - filename_start;
                    char *filename = XNMALLOC (filename_length + 1, char);

                    memcpy (filename, filename_start, filename_length);
                    filename[filename_length] = '\0';

                    po_callback_comment_filepos (filename, n);

                    free (filename);

                    continue;
                  }
                }
              }
          }

          /* Parsed a file comment without line number.  */
          {
            size_t filename_length = filename_end - filename_start;
            char *filename = XNMALLOC (filename_length + 1, char);

            memcpy (filename, filename_start, filename_length);
            filename[filename_length] = '\0';

            po_callback_comment_filepos (filename, (size_t)(-1));

            free (filename);
          }
        }
    }
}


/* Parse a SunOS or Solaris style file comment.
   Syntax of SunOS style:
     FILE_KEYWORD COLON STRING COMMA LINE_KEYWORD COLON NUMBER
   Syntax of Solaris style:
     FILE_KEYWORD COLON STRING COMMA LINE_KEYWORD NUMBER_KEYWORD COLON NUMBER
   where
     FILE_KEYWORD ::= "file" | "File"
     COLON ::= ":"
     COMMA ::= ","
     LINE_KEYWORD ::= "line"
     NUMBER_KEYWORD ::= "number"
     NUMBER ::= [0-9]+
   Return true if parsed, false if not a comment of this form. */
static bool
po_parse_comment_solaris_filepos (const char *s)
{
  if (s[0] == ' '
      && (s[1] == 'F' || s[1] == 'f')
      && s[2] == 'i' && s[3] == 'l' && s[4] == 'e'
      && s[5] == ':')
    {
      const char *string_start;
      const char *string_end;

      {
        const char *p = s + 6;

        while (*p == ' ' || *p == '\t')
          p++;
        string_start = p;
      }

      for (string_end = string_start; *string_end != '\0'; string_end++)
        {
          const char *p = string_end;

          while (*p == ' ' || *p == '\t')
            p++;

          if (*p == ',')
            {
              p++;

              while (*p == ' ' || *p == '\t')
                p++;

              if (p[0] == 'l' && p[1] == 'i' && p[2] == 'n' && p[3] == 'e')
                {
                  p += 4;

                  while (*p == ' ' || *p == '\t')
                    p++;

                  if (p[0] == 'n' && p[1] == 'u' && p[2] == 'm'
                      && p[3] == 'b' && p[4] == 'e' && p[5] == 'r')
                    {
                      p += 6;
                      while (*p == ' ' || *p == '\t')
                        p++;
                    }

                  if (*p == ':')
                    {
                      p++;

                      if (*p >= '0' && *p <= '9')
                        {
                          /* Accumulate a number.  */
                          size_t n = 0;

                          do
                            {
                              n = n * 10 + (*p - '0');
                              p++;
                            }
                          while (*p >= '0' && *p <= '9');

                          while (*p == ' ' || *p == '\t' || *p == '\n')
                            p++;

                          if (*p == '\0')
                            {
                              /* Parsed a Sun style file comment.  */
                              size_t string_length = string_end - string_start;
                              char *string =
                                XNMALLOC (string_length + 1, char);

                              memcpy (string, string_start, string_length);
                              string[string_length] = '\0';

                              po_callback_comment_filepos (string, n);

                              free (string);
                              return true;
                            }
                        }
                    }
                }
            }
        }
    }

  return false;
}


/* This function is called by po_gram_lex() whenever a comment is
   seen.  It analyzes the comment to see what sort it is, and then
   dispatches it to the appropriate method: call_comment, call_comment_dot,
   call_comment_filepos (via po_parse_comment_filepos), or
   call_comment_special.  */
void
po_callback_comment_dispatcher (const char *s)
{
  if (*s == '.')
    {
      s++;
      /* There is usually a space before the comment.  People don't
         consider it part of the comment, therefore remove it here.  */
      if (*s == ' ')
        s++;
      po_callback_comment_dot (s);
    }
  else if (*s == ':')
    {
      /* Parse the file location string.  The appropriate callback will be
         invoked.  */
      po_parse_comment_filepos (s + 1);
    }
  else if (*s == ',' || *s == '!')
    {
      /* Get all entries in the special comment line.  */
      po_callback_comment_special (s + 1);
    }
  else
    {
      /* It looks like a plain vanilla comment, but Solaris-style file
         position lines do, too.  Try to parse the lot.  If the parse
         succeeds, the appropriate callback will be invoked.  */
      if (po_parse_comment_solaris_filepos (s))
        /* Do nothing, it is a Sun-style file pos line.  */ ;
      else
        {
          /* There is usually a space before the comment.  People don't
             consider it part of the comment, therefore remove it here.  */
          if (*s == ' ')
            s++;
          po_callback_comment (s);
        }
    }
}
