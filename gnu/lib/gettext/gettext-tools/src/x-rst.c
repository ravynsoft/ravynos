/* xgettext RST/RSJ backend.
   Copyright (C) 2001-2003, 2005-2009, 2018-2019 Free Software Foundation, Inc.

   This file was written by Bruno Haible <haible@clisp.cons.org>, 2001.

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
#include "x-rst.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "c-ctype.h"
#include "po-charset.h"
#include "message.h"
#include "xgettext.h"
#include "xg-pos.h"
#include "xg-encoding.h"
#include "xg-mixed-string.h"
#include "xg-message.h"
#include "error.h"
#include "error-progname.h"
#include "xalloc.h"
#include "gettext.h"

#define _(s) gettext(s)

/* RST stands for Resource String Table.

   An RST file consists of several string definitions.  A string definition
   starts at the beginning of a line and looks like this:
       ModuleName.ConstName=StringExpression
   A StringExpression consists of string pieces of the form 'xyz',
   single characters of the form #nnn (decimal integer), and +
   at the end of the line to designate continuation on the next line.
   String definitions can be separated by blank lines or comment lines
   beginning with '#'.

   This backend attempts to be functionally equivalent to the 'rstconv'
   program, part of the Free Pascal run time library, written by
   Sebastian Guenther.  Except that
     * the locations are output as "ModuleName.ConstName",
       not "ModuleName:ConstName",
     * we add the flag '#, object-pascal-format' where appropriate.
 */

void
extract_rst (FILE *f,
             const char *real_filename, const char *logical_filename,
             flag_context_list_table_ty *flag_table,
             msgdomain_list_ty *mdlp)
{
  static char *buffer;
  static int bufmax;
  message_list_ty *mlp = mdlp->item[0]->messages;
  int line_number;

  line_number = 1;
  for (;;)
    {
      int c;
      int bufpos;
      char *location;
      char *msgid;
      lex_pos_ty pos;

      c = getc (f);
      if (c == EOF)
        break;

      /* Ignore blank line.  */
      if (c == '\n')
        {
          line_number++;
          continue;
        }

      /* Ignore comment line.  */
      if (c == '#')
        {
          do
            c = getc (f);
          while (c != EOF && c != '\n');
          if (c == EOF)
            break;
          line_number++;
          continue;
        }

      /* Read ModuleName.ConstName.  */
      bufpos = 0;
      for (;;)
        {
          if (c == EOF || c == '\n')
            {
              error_with_progname = false;
              error (EXIT_FAILURE, 0, _("%s:%d: invalid string definition"),
                     logical_filename, line_number);
              error_with_progname = true;
            }
          if (bufpos >= bufmax)
            {
              bufmax = 2 * bufmax + 10;
              buffer = xrealloc (buffer, bufmax);
            }
          if (c == '=')
            break;
          buffer[bufpos++] = c;
          c = getc (f);
          if (c == EOF && ferror (f))
            goto bomb;
        }
      buffer[bufpos] = '\0';
      location = xstrdup (buffer);

      /* Read StringExpression.  */
      bufpos = 0;
      for (;;)
        {
          c = getc (f);
          if (c == EOF)
            break;
          else if (c == '\n')
            {
              line_number++;
              break;
            }
          else if (c == '\'')
            {
              for (;;)
                {
                  c = getc (f);
                  /* Embedded single quotes like 'abc''def' don't occur.
                     See fpc-1.0.4/compiler/cresstr.pas.  */
                  if (c == EOF || c == '\n' || c == '\'')
                    break;
                  if (bufpos >= bufmax)
                    {
                      bufmax = 2 * bufmax + 10;
                      buffer = xrealloc (buffer, bufmax);
                    }
                  buffer[bufpos++] = c;
                }
              if (c == EOF)
                break;
              else if (c == '\n')
                {
                  line_number++;
                  break;
                }
            }
          else if (c == '#')
            {
              int n;
              c = getc (f);
              if (c == EOF && ferror (f))
                goto bomb;
              if (c == EOF || !c_isdigit (c))
                {
                  error_with_progname = false;
                  error (EXIT_FAILURE, 0, _("%s:%d: missing number after #"),
                         logical_filename, line_number);
                  error_with_progname = true;
                }
              n = (c - '0');
              for (;;)
                {
                  c = getc (f);
                  if (c == EOF || !c_isdigit (c))
                    break;
                  n = n * 10 + (c - '0');
                }
              if (bufpos >= bufmax)
                {
                  bufmax = 2 * bufmax + 10;
                  buffer = xrealloc (buffer, bufmax);
                }
              buffer[bufpos++] = (unsigned char) n;
              if (c == EOF)
                break;
              ungetc (c, f);
            }
          else if (c == '+')
            {
              c = getc (f);
              if (c == EOF)
                break;
              if (c == '\n')
                line_number++;
              else
                ungetc (c, f);
            }
          else
            {
              error_with_progname = false;
              error (EXIT_FAILURE, 0, _("%s:%d: invalid string expression"),
                     logical_filename, line_number);
              error_with_progname = true;
            }
        }
      if (bufpos >= bufmax)
        {
          bufmax = 2 * bufmax + 10;
          buffer = xrealloc (buffer, bufmax);
        }
      buffer[bufpos] = '\0';
      msgid = xstrdup (buffer);

      pos.file_name = location;
      pos.line_number = (size_t)(-1);

      remember_a_message (mlp, NULL, msgid, false, false, null_context, &pos,
                          NULL, NULL, false);

      /* Here c is the last read character: EOF or '\n'.  */
      if (c == EOF)
        break;
    }

  if (ferror (f))
    {
    bomb:
      error (EXIT_FAILURE, errno, _("error while reading \"%s\""),
             real_filename);
    }
}


/* RSJ stands for Resource String Table in JSON.

   An RSJ file is a JSON file that contains several string definitions.
   It has the format (modulo whitespace)
     {
       "version": 1,
       "strings":
         [
           {
             "hash": <integer>,
             "name": <string>,
             "sourcebytes": [ <integer>... ],
             "value": <string>
           },
           ...
         ]
     }
   The sourcebytes array contains the original source bytes, in the
   source encoding (not guaranteed to be ISO-8859-1, see
   <http://wiki.freepascal.org/FPC_Unicode_support#Source_file_codepage>).

   This backend attempts to be functionally equivalent to the 'rstconv'
   program, part of the Free Pascal run time library, written by
   Sebastian Guenther.  Except that
     * we use the "value" as msgid, not the "sourcebytes",
     * the locations are output as "ModuleName.ConstName",
       not "ModuleName:ConstName",
     * we add the flag '#, object-pascal-format' where appropriate.
 */

/* For the JSON syntax, refer to RFC 8259.  */

/* ======================== Reading of characters.  ======================== */

/* The input file stream.  */
static FILE *fp;


/* 1. line_number handling.  */

static int
phase1_getc ()
{
  int c = getc (fp);

  if (c == EOF)
    {
      if (ferror (fp))
        error (EXIT_FAILURE, errno, _("error while reading \"%s\""),
               real_file_name);
      return EOF;
    }

  if (c == '\n')
    line_number++;

  return c;
}

/* Supports only one pushback character.  */
static void
phase1_ungetc (int c)
{
  if (c != EOF)
    {
      if (c == '\n')
        --line_number;

      ungetc (c, fp);
    }
}


/* 2. Skipping whitespace.  */

/* Tests whether a phase1_getc() result is JSON whitespace.  */
static inline bool
is_whitespace (int c)
{
  return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

static int
phase2_getc ()
{
  int c;

  do
    c = phase1_getc ();
  while (is_whitespace (c));

  return c;
}

static void
phase2_ungetc (int c)
{
  phase1_ungetc (c);
}


/* ========================== Reading of tokens.  ========================== */

/* Result of parsing a token.  */

enum parse_result
{
  pr_parsed, /* successfully parsed */
  pr_none,   /* the next token is of a different type */
  pr_syntax  /* syntax error inside the token */
};

static char *buffer;
static int bufmax;

/* Parses an integer.  Returns it in buffer, of length bufmax.
   Returns pr_parsed or pr_none.  */
static enum parse_result
parse_integer ()
{
  int c;
  int bufpos;

  c = phase2_getc ();
  bufpos = 0;
  for (;;)
    {
      if (bufpos >= bufmax)
        {
          bufmax = 2 * bufmax + 10;
          buffer = xrealloc (buffer, bufmax);
        }
      if (!(c >= '0' && c <= '9'))
        break;
      buffer[bufpos++] = c;
      c = phase1_getc ();
    }
  phase1_ungetc (c);
  buffer[bufpos] = '\0';
  return (bufpos == 0 ? pr_none : pr_parsed);
}

static struct mixed_string_buffer stringbuf;

/* Parses a string.  Returns it in stringbuf, in UTF-8 encoding.
   Returns a parse_result.  */
static enum parse_result
parse_string ()
{
  int c;

  c = phase2_getc ();
  if (c != '"')
    {
      phase2_ungetc (c);
      return pr_none;
    }
  mixed_string_buffer_init (&stringbuf, lc_string,
                            logical_file_name, line_number);
  for (;;)
    {
      c = phase1_getc ();
      /* Keep line_number in sync.  */
      stringbuf.line_number = line_number;
      if (c == EOF || (c >= 0 && c < 0x20))
        return pr_syntax;
      if (c == '"')
        break;
      if (c == '\\')
        {
          c = phase1_getc ();
          if (c == 'u')
            {
              unsigned int n = 0;
              int i;

              for (i = 0; i < 4; i++)
                {
                  c = phase1_getc ();

                  if (c >= '0' && c <= '9')
                    n = (n << 4) + (c - '0');
                  else if (c >= 'A' && c <= 'F')
                    n = (n << 4) + (c - 'A' + 10);
                  else if (c >= 'a' && c <= 'f')
                    n = (n << 4) + (c - 'a' + 10);
                  else
                    return pr_syntax;
                }
              mixed_string_buffer_append_unicode (&stringbuf, n);
            }
          else
            {
              switch (c)
                {
                case '"':
                case '\\':
                case '/':
                  break;
                case 'b':
                  c = '\b';
                  break;
                case 'f':
                  c = '\f';
                  break;
                case 'n':
                  c = '\n';
                  break;
                case 'r':
                  c = '\r';
                  break;
                case 't':
                  c = '\t';
                  break;
                default:
                  return pr_syntax;
                }
              mixed_string_buffer_append_char (&stringbuf, c);
            }
        }
      else
        mixed_string_buffer_append_char (&stringbuf, c);
    }
  return pr_parsed;
}

void
extract_rsj (FILE *f,
             const char *real_filename, const char *logical_filename,
             flag_context_list_table_ty *flag_table,
             msgdomain_list_ty *mdlp)
{
  message_list_ty *mlp = mdlp->item[0]->messages;
  int c;

  fp = f;
  real_file_name = real_filename;
  logical_file_name = xstrdup (logical_filename);
  line_number = 1;

  /* JSON is always in UTF-8.  */
  xgettext_current_source_encoding = po_charset_utf8;

  /* Parse the initial opening brace.  */
  c = phase2_getc ();
  if (c != '{')
    goto invalid_json;

  c = phase2_getc ();
  if (c != '}')
    {
      phase2_ungetc (c);
      for (;;)
        {
          /* Parse a string.  */
          char *s1;
          if (parse_string () != pr_parsed)
            goto invalid_json;
          s1 = mixed_string_contents_free1 (
                 mixed_string_buffer_result (&stringbuf));

          /* Parse a colon.  */
          c = phase2_getc ();
          if (c != ':')
            goto invalid_json;

          if (strcmp (s1, "version") == 0)
            {
              /* Parse an integer.  */
              if (parse_integer () != pr_parsed)
                goto invalid_rsj;
              if (strcmp (buffer, "1") != 0)
                goto invalid_rsj_version;
            }
          else if (strcmp (s1, "strings") == 0)
            {
              /* Parse an array.  */
              c = phase2_getc ();
              if (c != '[')
                goto invalid_rsj;

              c = phase2_getc ();
              if (c != ']')
                {
                  phase2_ungetc (c);
                  for (;;)
                    {
                      char *location = NULL;
                      char *msgid = NULL;
                      lex_pos_ty pos;

                      /* Parse an object.  */
                      c = phase2_getc ();
                      if (c != '{')
                        goto invalid_rsj;

                      c = phase2_getc ();
                      if (c != '}')
                        {
                          phase2_ungetc (c);
                          for (;;)
                            {
                              /* Parse a string.  */
                              char *s2;
                              if (parse_string () != pr_parsed)
                                goto invalid_json;
                              s2 = mixed_string_contents_free1 (
                                     mixed_string_buffer_result (&stringbuf));

                              /* Parse a colon.  */
                              c = phase2_getc ();
                              if (c != ':')
                                goto invalid_json;

                              if (strcmp (s2, "hash") == 0)
                                {
                                  /* Parse an integer.  */
                                  if (parse_integer () != pr_parsed)
                                    goto invalid_rsj;
                                }
                              else if (strcmp (s2, "name") == 0)
                                {
                                  /* Parse a string.  */
                                  enum parse_result r = parse_string ();
                                  if (r == pr_none)
                                    goto invalid_rsj;
                                  if (r == pr_syntax || location != NULL)
                                    goto invalid_json;
                                  location =
                                    mixed_string_contents_free1 (
                                      mixed_string_buffer_result (&stringbuf));
                                }
                              else if (strcmp (s2, "sourcebytes") == 0)
                                {
                                  /* Parse an array.  */
                                  c = phase2_getc ();
                                  if (c != '[')
                                    goto invalid_rsj;

                                  c = phase2_getc ();
                                  if (c != ']')
                                    {
                                      phase2_ungetc (c);
                                      for (;;)
                                        {
                                          /* Parse an integer.  */
                                          if (parse_integer () != pr_parsed)
                                            goto invalid_rsj;

                                          /* Parse a comma.  */
                                          c = phase2_getc ();
                                          if (c == ']')
                                            break;
                                          if (c != ',')
                                            goto invalid_json;
                                        }
                                    }
                                }
                              else if (strcmp (s2, "value") == 0)
                                {
                                  /* Parse a string.  */
                                  enum parse_result r = parse_string ();
                                  if (r == pr_none)
                                    goto invalid_rsj;
                                  if (r == pr_syntax || msgid != NULL)
                                    goto invalid_json;
                                  msgid =
                                    mixed_string_contents_free1 (
                                      mixed_string_buffer_result (&stringbuf));
                                }
                              else
                                goto invalid_rsj;

                              free (s2);

                              /* Parse a comma.  */
                              c = phase2_getc ();
                              if (c == '}')
                                break;
                              if (c != ',')
                                goto invalid_json;
                            }
                        }

                      if (location == NULL || msgid == NULL)
                        goto invalid_rsj;

                      pos.file_name = location;
                      pos.line_number = (size_t)(-1);

                      remember_a_message (mlp, NULL, msgid, true, false,
                                          null_context, &pos,
                                          NULL, NULL, false);

                      /* Parse a comma.  */
                      c = phase2_getc ();
                      if (c == ']')
                        break;
                      if (c != ',')
                        goto invalid_json;
                    }
                }
            }
          else
            goto invalid_rsj;

          /* Parse a comma.  */
          c = phase2_getc ();
          if (c == '}')
            break;
          if (c != ',')
            goto invalid_json;
        }
    }

  /* Seen the closing brace.  */
  c = phase2_getc ();
  if (c != EOF)
    goto invalid_json;

  fp = NULL;
  real_file_name = NULL;
  logical_file_name = NULL;
  line_number = 0;

  return;

 invalid_json:
  error_with_progname = false;
  error (EXIT_FAILURE, 0, _("%s:%d: invalid JSON syntax"),
         logical_filename, line_number);
  error_with_progname = true;
  return;

 invalid_rsj:
  error_with_progname = false;
  error (EXIT_FAILURE, 0, _("%s:%d: invalid RSJ syntax"),
         logical_filename, line_number);
  error_with_progname = true;
  return;

 invalid_rsj_version:
  error_with_progname = false;
  error (EXIT_FAILURE, 0,
         _("%s:%d: invalid RSJ version. Only version 1 is supported."),
         logical_filename, line_number);
  error_with_progname = true;
  return;
}
