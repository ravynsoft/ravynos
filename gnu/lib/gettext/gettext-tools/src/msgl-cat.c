/* Message list concatenation and duplicate handling.
   Copyright (C) 2001-2003, 2005-2008, 2012, 2015, 2019-2021, 2023 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

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
#include <alloca.h>

/* Specification.  */
#include "msgl-cat.h"

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "xerror.h"
#include "xvasprintf.h"
#include "message.h"
#include "read-catalog.h"
#include "po-charset.h"
#include "msgl-ascii.h"
#include "msgl-ofn.h"
#include "msgl-equal.h"
#include "msgl-iconv.h"
#include "xalloc.h"
#include "xmalloca.h"
#include "c-strstr.h"
#include "basename-lgpl.h"
#include "gettext.h"

#define _(str) gettext (str)


/* These variables control which messages are selected.  */
int more_than;
int less_than;

/* If true, use the first available translation.
   If false, merge all available translations into one and fuzzy it.  */
bool use_first;

/* If true, merge like msgcomm.
   If false, merge like msgcat and msguniq.  */
bool msgcomm_mode = false;

/* If true, omit the header entry.
   If false, keep the header entry present in the input.  */
bool omit_header = false;


static bool
is_message_selected (const message_ty *tmp)
{
  int used = (tmp->used >= 0 ? tmp->used : - tmp->used);

  return (is_header (tmp)
          ? !omit_header        /* keep the header entry */
          : (used > more_than && used < less_than));
}


static bool
is_message_needed (const message_ty *mp)
{
  if (!msgcomm_mode
      && ((!is_header (mp) && mp->is_fuzzy) || mp->msgstr[0] == '\0'))
    /* Weak translation.  Needed if there are only weak translations.  */
    return mp->tmp->used < 0 && is_message_selected (mp->tmp);
  else
    /* Good translation.  */
    return is_message_selected (mp->tmp);
}


/* The use_first logic.  */
static bool
is_message_first_needed (const message_ty *mp)
{
  if (mp->tmp->obsolete && is_message_needed (mp))
    {
      mp->tmp->obsolete = false;
      return true;
    }
  else
    return false;
}


msgdomain_list_ty *
catenate_msgdomain_list (string_list_ty *file_list,
                         catalog_input_format_ty input_syntax,
                         const char *to_code)
{
  const char * const *files = file_list->item;
  size_t nfiles = file_list->nitems;
  msgdomain_list_ty **mdlps;
  const char ***canon_charsets;
  const char ***identifications;
  msgdomain_list_ty *total_mdlp;
  const char *canon_to_code;
  size_t n, j;

  /* Read input files.  */
  mdlps = XNMALLOC (nfiles, msgdomain_list_ty *);
  for (n = 0; n < nfiles; n++)
    mdlps[n] = read_catalog_file (files[n], input_syntax);

  /* Determine the canonical name of each input file's encoding.  */
  canon_charsets = XNMALLOC (nfiles, const char **);
  for (n = 0; n < nfiles; n++)
    {
      msgdomain_list_ty *mdlp = mdlps[n];
      size_t k;

      canon_charsets[n] = XNMALLOC (mdlp->nitems, const char *);
      for (k = 0; k < mdlp->nitems; k++)
        {
          message_list_ty *mlp = mdlp->item[k]->messages;
          const char *canon_from_code = NULL;

          if (mlp->nitems > 0)
            {
              for (j = 0; j < mlp->nitems; j++)
                if (is_header (mlp->item[j]) && !mlp->item[j]->obsolete)
                  {
                    const char *header = mlp->item[j]->msgstr;

                    if (header != NULL)
                      {
                        const char *charsetstr = c_strstr (header, "charset=");

                        if (charsetstr != NULL)
                          {
                            size_t len;
                            char *charset;
                            const char *canon_charset;

                            charsetstr += strlen ("charset=");
                            len = strcspn (charsetstr, " \t\n");
                            charset = (char *) xmalloca (len + 1);
                            memcpy (charset, charsetstr, len);
                            charset[len] = '\0';

                            canon_charset = po_charset_canonicalize (charset);
                            if (canon_charset == NULL)
                              {
                                /* Don't give an error for POT files, because
                                   POT files usually contain only ASCII msgids.
                                   Also don't give an error for disguised POT
                                   files that actually contain only ASCII
                                   msgids.  */
                                const char *filename = files[n];
                                size_t filenamelen = strlen (filename);

                                if (strcmp (charset, "CHARSET") == 0
                                    && ((filenamelen >= 4
                                         && memcmp (filename + filenamelen - 4,
                                                    ".pot", 4) == 0)
                                        || is_ascii_message_list (mlp)))
                                  canon_charset = po_charset_ascii;
                                else
                                  error (EXIT_FAILURE, 0,
                                         _("present charset \"%s\" is not a portable encoding name"),
                                         charset);
                              }

                            freea (charset);

                            if (canon_from_code == NULL)
                              canon_from_code = canon_charset;
                            else if (canon_from_code != canon_charset)
                              error (EXIT_FAILURE, 0,
                                     _("two different charsets \"%s\" and \"%s\" in input file"),
                                     canon_from_code, canon_charset);
                          }
                      }
                  }
              if (canon_from_code == NULL)
                {
                  if (is_ascii_message_list (mlp))
                    canon_from_code = po_charset_ascii;
                  else if (mdlp->encoding != NULL)
                    canon_from_code = mdlp->encoding;
                  else
                    {
                      if (k == 0)
                        error (EXIT_FAILURE, 0,
                               _("input file '%s' doesn't contain a header entry with a charset specification"),
                               files[n]);
                      else
                        error (EXIT_FAILURE, 0,
                               _("domain \"%s\" in input file '%s' doesn't contain a header entry with a charset specification"),
                               mdlp->item[k]->domain, files[n]);
                    }
                }
            }
          canon_charsets[n][k] = canon_from_code;
        }
    }

  /* Determine textual identifications of each file/domain combination.  */
  identifications = XNMALLOC (nfiles, const char **);
  for (n = 0; n < nfiles; n++)
    {
      const char *filename = last_component (files[n]);
      msgdomain_list_ty *mdlp = mdlps[n];
      size_t k;

      identifications[n] = XNMALLOC (mdlp->nitems, const char *);
      for (k = 0; k < mdlp->nitems; k++)
        {
          const char *domain = mdlp->item[k]->domain;
          message_list_ty *mlp = mdlp->item[k]->messages;
          char *project_id = NULL;

          for (j = 0; j < mlp->nitems; j++)
            if (is_header (mlp->item[j]) && !mlp->item[j]->obsolete)
              {
                const char *header = mlp->item[j]->msgstr;

                if (header != NULL)
                  {
                    const char *cp = c_strstr (header, "Project-Id-Version:");

                    if (cp != NULL)
                      {
                        const char *endp;

                        cp += sizeof ("Project-Id-Version:") - 1;

                        endp = strchr (cp, '\n');
                        if (endp == NULL)
                          endp = cp + strlen (cp);

                        while (cp < endp && *cp == ' ')
                          cp++;

                        if (cp < endp)
                          {
                            size_t len = endp - cp;
                            project_id = XNMALLOC (len + 1, char);
                            memcpy (project_id, cp, len);
                            project_id[len] = '\0';
                          }
                        break;
                      }
                  }
              }

          identifications[n][k] =
            (project_id != NULL
             ? (k > 0 ? xasprintf ("%s:%s (%s)", filename, domain, project_id)
                      : xasprintf ("%s (%s)", filename, project_id))
             : (k > 0 ? xasprintf ("%s:%s", filename, domain)
                      : xasprintf ("%s", filename)));
        }
    }

  /* Create list of resulting messages, but don't fill it.  Only count
     the number of translations for each message.
     If for a message, there is at least one non-fuzzy, non-empty translation,
     use only the non-fuzzy, non-empty translations.  Otherwise use the
     fuzzy or empty translations as well.  */
  total_mdlp = msgdomain_list_alloc (true);
  for (n = 0; n < nfiles; n++)
    {
      msgdomain_list_ty *mdlp = mdlps[n];
      size_t k;

      for (k = 0; k < mdlp->nitems; k++)
        {
          const char *domain = mdlp->item[k]->domain;
          message_list_ty *mlp = mdlp->item[k]->messages;
          message_list_ty *total_mlp;

          total_mlp = msgdomain_list_sublist (total_mdlp, domain, true);

          for (j = 0; j < mlp->nitems; j++)
            {
              message_ty *mp = mlp->item[j];
              message_ty *tmp;
              size_t i;

              tmp = message_list_search (total_mlp, mp->msgctxt, mp->msgid);
              if (tmp != NULL)
                {
                  if ((tmp->msgid_plural != NULL) != (mp->msgid_plural != NULL))
                    {
                      char *errormsg =
                        xasprintf (_("msgid '%s' is used without plural and with plural."),
                                   mp->msgid);
                      multiline_error (xstrdup (""),
                                       xasprintf ("%s\n", errormsg));
                    }
                }
              else
                {
                  tmp = message_alloc (mp->msgctxt, mp->msgid, mp->msgid_plural,
                                       NULL, 0, &mp->pos);
                  tmp->is_fuzzy = true; /* may be set to false later */
                  for (i = 0; i < NFORMATS; i++)
                    tmp->is_format[i] = undecided; /* may be set to yes/no later */
                  tmp->range.min = - INT_MAX;
                  tmp->range.max = - INT_MAX;
                  tmp->do_wrap = yes; /* may be set to no later */
                  for (i = 0; i < NSYNTAXCHECKS; i++)
                    tmp->do_syntax_check[i] = undecided; /* may be set to yes/no later */
                  tmp->obsolete = true; /* may be set to false later */
                  tmp->alternative_count = 0;
                  tmp->alternative = NULL;
                  message_list_append (total_mlp, tmp);
                }

              if (!msgcomm_mode
                  && ((!is_header (mp) && mp->is_fuzzy)
                      || mp->msgstr[0] == '\0'))
                /* Weak translation.  Counted as negative tmp->used.  */
                {
                  if (tmp->used <= 0)
                    tmp->used--;
                }
              else
                /* Good translation.  Counted as positive tmp->used.  */
                {
                  if (tmp->used < 0)
                    tmp->used = 0;
                  tmp->used++;
                }
              mp->tmp = tmp;
            }
        }
    }

  /* Remove messages that are not used and need not be converted.  */
  for (n = 0; n < nfiles; n++)
    {
      msgdomain_list_ty *mdlp = mdlps[n];
      size_t k;

      for (k = 0; k < mdlp->nitems; k++)
        {
          message_list_ty *mlp = mdlp->item[k]->messages;

          message_list_remove_if_not (mlp,
                                      use_first
                                      ? is_message_first_needed
                                      : is_message_needed);

          /* If no messages are remaining, drop the charset.  */
          if (mlp->nitems == 0)
            canon_charsets[n][k] = NULL;
        }
    }
  {
    size_t k;

    for (k = 0; k < total_mdlp->nitems; k++)
      {
        message_list_ty *mlp = total_mdlp->item[k]->messages;

        message_list_remove_if_not (mlp, is_message_selected);
      }
  }

  /* Determine the common known a-priori encoding, if any.  */
  if (nfiles > 0)
    {
      bool all_same_encoding = true;

      for (n = 1; n < nfiles; n++)
        if (mdlps[n]->encoding != mdlps[0]->encoding)
          {
            all_same_encoding = false;
            break;
          }

      if (all_same_encoding)
        total_mdlp->encoding = mdlps[0]->encoding;
    }

  /* Determine whether we need a target encoding that contains the control
     characters needed for escaping file names with spaces.  */
  bool has_filenames_with_spaces = false;
  for (n = 0; n < nfiles; n++)
    {
      has_filenames_with_spaces =
        has_filenames_with_spaces
        || msgdomain_list_has_filenames_with_spaces (mdlps[n]);
    }

  /* Determine the target encoding for the remaining messages.  */
  if (to_code != NULL)
    {
      /* Canonicalize target encoding.  */
      canon_to_code = po_charset_canonicalize (to_code);
      if (canon_to_code == NULL)
        error (EXIT_FAILURE, 0,
               _("target charset \"%s\" is not a portable encoding name."),
               to_code);
      /* Test whether the control characters required for escaping file names
         with spaces are present in the target encoding.  */
      if (has_filenames_with_spaces
          && !(canon_to_code == po_charset_utf8
               || strcmp (canon_to_code, "GB18030") == 0))
        error (EXIT_FAILURE, 0,
               _("Cannot write the control characters that protect file names with spaces in the %s encoding"),
               canon_to_code);
    }
  else
    {
      /* No target encoding was specified.  Test whether the messages are
         all in a single encoding.  If so and if !has_filenames_with_spaces,
         conversion is not needed.  */
      const char *first = NULL;
      const char *second = NULL;
      bool with_ASCII = false;
      bool with_UTF8 = false;
      bool all_ASCII_compatible = true;

      for (n = 0; n < nfiles; n++)
        {
          msgdomain_list_ty *mdlp = mdlps[n];
          size_t k;

          for (k = 0; k < mdlp->nitems; k++)
            if (canon_charsets[n][k] != NULL)
              {
                if (canon_charsets[n][k] == po_charset_ascii)
                  with_ASCII = true;
                else
                  {
                    if (first == NULL)
                      first = canon_charsets[n][k];
                    else if (canon_charsets[n][k] != first && second == NULL)
                      second = canon_charsets[n][k];

                    if (strcmp (canon_charsets[n][k], "UTF-8") == 0)
                      with_UTF8 = true;

                    if (!po_charset_ascii_compatible (canon_charsets[n][k]))
                      all_ASCII_compatible = false;
                  }
              }
        }

      if (with_ASCII && !all_ASCII_compatible)
        {
          /* assert (first != NULL); */
          if (second == NULL)
            second = po_charset_ascii;
        }

      if (second != NULL)
        {
          /* A conversion is needed.  Warn the user since he hasn't asked
             for it and might be surprised.  */
          if (with_UTF8)
            multiline_warning (xasprintf (_("warning: ")),
                               xasprintf (_("\
Input files contain messages in different encodings, UTF-8 among others.\n\
Converting the output to UTF-8.\n\
")));
          else
            multiline_warning (xasprintf (_("warning: ")),
                               xasprintf (_("\
Input files contain messages in different encodings, %s and %s among others.\n\
Converting the output to UTF-8.\n\
To select a different output encoding, use the --to-code option.\n\
"), first, second));
          canon_to_code = po_charset_utf8;
        }
      else if (has_filenames_with_spaces)
        {
          /* A conversion is needed.  Warn the user since he hasn't asked
             for it and might be surprised.  */
          if (first != NULL
              && (first == po_charset_utf8 || strcmp (first, "GB18030") == 0))
            canon_to_code = first;
          else
            canon_to_code = po_charset_utf8;
          multiline_warning (xasprintf (_("warning: ")),
                             xasprintf (_("\
Input files contain messages referenced in file names with spaces.\n\
Converting the output to %s.\n\
"),
                                        canon_to_code));
        }
      else if (first != NULL && with_ASCII && all_ASCII_compatible)
        {
          /* The conversion is a no-op conversion.  Don't warn the user,
             but still perform the conversion, in order to check that the
             input was really ASCII.  */
          canon_to_code = first;
        }
      else
        {
          /* No conversion needed.  */
          canon_to_code = NULL;
        }
    }

  /* Now convert the remaining messages to canon_to_code.  */
  if (canon_to_code != NULL)
    for (n = 0; n < nfiles; n++)
      {
        msgdomain_list_ty *mdlp = mdlps[n];
        size_t k;

        for (k = 0; k < mdlp->nitems; k++)
          if (canon_charsets[n][k] != NULL)
            /* If the user hasn't given a to_code, don't bother doing a noop
               conversion that would only replace the charset name in the
               header entry with its canonical equivalent.  */
            if (!(to_code == NULL && canon_charsets[n][k] == canon_to_code))
              if (iconv_message_list (mdlp->item[k]->messages,
                                      canon_charsets[n][k], canon_to_code,
                                      files[n]))
                {
                  multiline_error (xstrdup (""),
                                   xasprintf (_("\
Conversion of file %s from %s encoding to %s encoding\n\
changes some msgids or msgctxts.\n\
Either change all msgids and msgctxts to be pure ASCII, or ensure they are\n\
UTF-8 encoded from the beginning, i.e. already in your source code files.\n"),
                                              files[n], canon_charsets[n][k],
                                              canon_to_code));
                  exit (EXIT_FAILURE);
                }
      }

  /* Fill the resulting messages.  */
  for (n = 0; n < nfiles; n++)
    {
      msgdomain_list_ty *mdlp = mdlps[n];
      size_t k;

      for (k = 0; k < mdlp->nitems; k++)
        {
          message_list_ty *mlp = mdlp->item[k]->messages;

          for (j = 0; j < mlp->nitems; j++)
            {
              message_ty *mp = mlp->item[j];
              message_ty *tmp = mp->tmp;
              size_t i;

              /* No need to discard unneeded weak translations here;
                 they have already been filtered out above.  */
              if (use_first || tmp->used == 1 || tmp->used == -1)
                {
                  /* Copy mp, as only message, into tmp.  */
                  tmp->msgstr = mp->msgstr;
                  tmp->msgstr_len = mp->msgstr_len;
                  tmp->pos = mp->pos;
                  if (mp->comment)
                    for (i = 0; i < mp->comment->nitems; i++)
                      message_comment_append (tmp, mp->comment->item[i]);
                  if (mp->comment_dot)
                    for (i = 0; i < mp->comment_dot->nitems; i++)
                      message_comment_dot_append (tmp,
                                                  mp->comment_dot->item[i]);
                  for (i = 0; i < mp->filepos_count; i++)
                    message_comment_filepos (tmp, mp->filepos[i].file_name,
                                             mp->filepos[i].line_number);
                  tmp->is_fuzzy = mp->is_fuzzy;
                  for (i = 0; i < NFORMATS; i++)
                    tmp->is_format[i] = mp->is_format[i];
                  tmp->range = mp->range;
                  tmp->do_wrap = mp->do_wrap;
                  for (i = 0; i < NSYNTAXCHECKS; i++)
                    tmp->do_syntax_check[i] = mp->do_syntax_check[i];
                  tmp->prev_msgctxt = mp->prev_msgctxt;
                  tmp->prev_msgid = mp->prev_msgid;
                  tmp->prev_msgid_plural = mp->prev_msgid_plural;
                  tmp->obsolete = mp->obsolete;
                }
              else if (msgcomm_mode)
                {
                  /* Copy mp, as only message, into tmp.  */
                  if (tmp->msgstr == NULL)
                    {
                      tmp->msgstr = mp->msgstr;
                      tmp->msgstr_len = mp->msgstr_len;
                      tmp->pos = mp->pos;
                      tmp->is_fuzzy = mp->is_fuzzy;
                      tmp->prev_msgctxt = mp->prev_msgctxt;
                      tmp->prev_msgid = mp->prev_msgid;
                      tmp->prev_msgid_plural = mp->prev_msgid_plural;
                    }
                  if (mp->comment && tmp->comment == NULL)
                    for (i = 0; i < mp->comment->nitems; i++)
                      message_comment_append (tmp, mp->comment->item[i]);
                  if (mp->comment_dot && tmp->comment_dot == NULL)
                    for (i = 0; i < mp->comment_dot->nitems; i++)
                      message_comment_dot_append (tmp,
                                                  mp->comment_dot->item[i]);
                  for (i = 0; i < mp->filepos_count; i++)
                    message_comment_filepos (tmp, mp->filepos[i].file_name,
                                             mp->filepos[i].line_number);
                  for (i = 0; i < NFORMATS; i++)
                    if (tmp->is_format[i] == undecided)
                      tmp->is_format[i] = mp->is_format[i];
                  if (tmp->range.min == - INT_MAX
                      && tmp->range.max == - INT_MAX)
                    tmp->range = mp->range;
                  else if (has_range_p (mp->range) && has_range_p (tmp->range))
                    {
                      if (mp->range.min < tmp->range.min)
                        tmp->range.min = mp->range.min;
                      if (mp->range.max > tmp->range.max)
                        tmp->range.max = mp->range.max;
                    }
                  else
                    {
                      tmp->range.min = -1;
                      tmp->range.max = -1;
                    }
                  if (tmp->do_wrap == undecided)
                    tmp->do_wrap = mp->do_wrap;
                  for (i = 0; i < NSYNTAXCHECKS; i++)
                    if (tmp->do_syntax_check[i] == undecided)
                      tmp->do_syntax_check[i] = mp->do_syntax_check[i];
                  tmp->obsolete = false;
                }
              else
                {
                  /* Copy mp, among others, into tmp.  */
                  char *id = xasprintf ("#-#-#-#-#  %s  #-#-#-#-#",
                                        identifications[n][k]);
                  size_t nbytes;

                  if (tmp->alternative_count == 0)
                    tmp->pos = mp->pos;

                  i = tmp->alternative_count;
                  nbytes = (i + 1) * sizeof (struct altstr);
                  tmp->alternative = xrealloc (tmp->alternative, nbytes);
                  tmp->alternative[i].msgstr = mp->msgstr;
                  tmp->alternative[i].msgstr_len = mp->msgstr_len;
                  tmp->alternative[i].msgstr_end =
                    tmp->alternative[i].msgstr + tmp->alternative[i].msgstr_len;
                  tmp->alternative[i].comment = mp->comment;
                  tmp->alternative[i].comment_dot = mp->comment_dot;
                  tmp->alternative[i].id = id;
                  tmp->alternative_count = i + 1;

                  for (i = 0; i < mp->filepos_count; i++)
                    message_comment_filepos (tmp, mp->filepos[i].file_name,
                                             mp->filepos[i].line_number);
                  if (!mp->is_fuzzy)
                    tmp->is_fuzzy = false;
                  for (i = 0; i < NFORMATS; i++)
                    if (mp->is_format[i] == yes)
                      tmp->is_format[i] = yes;
                    else if (mp->is_format[i] == no
                             && tmp->is_format[i] == undecided)
                      tmp->is_format[i] = no;
                  if (tmp->range.min == - INT_MAX
                      && tmp->range.max == - INT_MAX)
                    tmp->range = mp->range;
                  else if (has_range_p (mp->range) && has_range_p (tmp->range))
                    {
                      if (mp->range.min < tmp->range.min)
                        tmp->range.min = mp->range.min;
                      if (mp->range.max > tmp->range.max)
                        tmp->range.max = mp->range.max;
                    }
                  else
                    {
                      tmp->range.min = -1;
                      tmp->range.max = -1;
                    }
                  if (mp->do_wrap == no)
                    tmp->do_wrap = no;
                  for (i = 0; i < NSYNTAXCHECKS; i++)
                    if (mp->do_syntax_check[i] == yes)
                      tmp->do_syntax_check[i] = yes;
                    else if (mp->do_syntax_check[i] == no
                             && tmp->do_syntax_check[i] == undecided)
                      tmp->do_syntax_check[i] = no;
                  /* Don't fill tmp->prev_msgid in this case.  */
                  if (!mp->obsolete)
                    tmp->obsolete = false;
                }
            }
        }
    }
  {
    size_t k;

    for (k = 0; k < total_mdlp->nitems; k++)
      {
        message_list_ty *mlp = total_mdlp->item[k]->messages;

        for (j = 0; j < mlp->nitems; j++)
          {
            message_ty *tmp = mlp->item[j];

            if (tmp->alternative_count > 0)
              {
                /* Test whether all alternative translations are equal.  */
                struct altstr *first = &tmp->alternative[0];
                size_t i;

                for (i = 0; i < tmp->alternative_count; i++)
                  if (!(tmp->alternative[i].msgstr_len == first->msgstr_len
                        && memcmp (tmp->alternative[i].msgstr, first->msgstr,
                                   first->msgstr_len) == 0))
                    break;

                if (i == tmp->alternative_count)
                  {
                    /* All alternatives are equal.  */
                    tmp->msgstr = first->msgstr;
                    tmp->msgstr_len = first->msgstr_len;
                  }
                else
                  {
                    /* Concatenate the alternative msgstrs into a single one,
                       separated by markers.  */
                    size_t len;
                    const char *p;
                    const char *p_end;
                    char *new_msgstr;
                    char *np;

                    len = 0;
                    for (i = 0; i < tmp->alternative_count; i++)
                      {
                        size_t id_len = strlen (tmp->alternative[i].id);

                        len += tmp->alternative[i].msgstr_len;

                        p = tmp->alternative[i].msgstr;
                        p_end = tmp->alternative[i].msgstr_end;
                        for (; p < p_end; p += strlen (p) + 1)
                          len += id_len + 2;
                      }

                    new_msgstr = XNMALLOC (len, char);
                    np = new_msgstr;
                    for (;;)
                      {
                        /* Test whether there's one more plural form to
                           process.  */
                        for (i = 0; i < tmp->alternative_count; i++)
                          if (tmp->alternative[i].msgstr
                              < tmp->alternative[i].msgstr_end)
                            break;
                        if (i == tmp->alternative_count)
                          break;

                        /* Process next plural form.  */
                        for (i = 0; i < tmp->alternative_count; i++)
                          if (tmp->alternative[i].msgstr
                              < tmp->alternative[i].msgstr_end)
                            {
                              if (np > new_msgstr && np[-1] != '\0'
                                  && np[-1] != '\n')
                                *np++ = '\n';

                              len = strlen (tmp->alternative[i].id);
                              memcpy (np, tmp->alternative[i].id, len);
                              np += len;
                              *np++ = '\n';

                              len = strlen (tmp->alternative[i].msgstr);
                              memcpy (np, tmp->alternative[i].msgstr, len);
                              np += len;
                              tmp->alternative[i].msgstr += len + 1;
                            }

                        /* Plural forms are separated by NUL bytes.  */
                        *np++ = '\0';
                      }
                    tmp->msgstr = new_msgstr;
                    tmp->msgstr_len = np - new_msgstr;

                    tmp->is_fuzzy = true;
                  }

                /* Test whether all alternative comments are equal.  */
                for (i = 0; i < tmp->alternative_count; i++)
                  if (tmp->alternative[i].comment == NULL
                      || !string_list_equal (tmp->alternative[i].comment,
                                             first->comment))
                    break;

                if (i == tmp->alternative_count)
                  /* All alternatives are equal.  */
                  tmp->comment = first->comment;
                else
                  /* Concatenate the alternative comments into a single one,
                     separated by markers.  */
                  for (i = 0; i < tmp->alternative_count; i++)
                    {
                      string_list_ty *slp = tmp->alternative[i].comment;

                      if (slp != NULL)
                        {
                          size_t l;

                          message_comment_append (tmp, tmp->alternative[i].id);
                          for (l = 0; l < slp->nitems; l++)
                            message_comment_append (tmp, slp->item[l]);
                        }
                    }

                /* Test whether all alternative dot comments are equal.  */
                for (i = 0; i < tmp->alternative_count; i++)
                  if (tmp->alternative[i].comment_dot == NULL
                      || !string_list_equal (tmp->alternative[i].comment_dot,
                                             first->comment_dot))
                    break;

                if (i == tmp->alternative_count)
                  /* All alternatives are equal.  */
                  tmp->comment_dot = first->comment_dot;
                else
                  /* Concatenate the alternative dot comments into a single one,
                     separated by markers.  */
                  for (i = 0; i < tmp->alternative_count; i++)
                    {
                      string_list_ty *slp = tmp->alternative[i].comment_dot;

                      if (slp != NULL)
                        {
                          size_t l;

                          message_comment_dot_append (tmp,
                                                      tmp->alternative[i].id);
                          for (l = 0; l < slp->nitems; l++)
                            message_comment_dot_append (tmp, slp->item[l]);
                        }
                    }
              }
          }
      }
  }

  return total_mdlp;
}
