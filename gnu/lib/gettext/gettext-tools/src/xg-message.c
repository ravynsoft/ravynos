/* Extracting a message.  Accumulating the message list.
   Copyright (C) 2001-2020, 2023 Free Software Foundation, Inc.

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
#include "xg-message.h"

#include <stdio.h>

#include "c-strstr.h"
#include "error-progname.h"
#include "format.h"
#include "read-catalog-abstract.h"
#include "xalloc.h"
#include "xerror.h"
#include "xvasprintf.h"
#include "verify.h"

#include "xgettext.h"

#include "gettext.h"
#define _(str) gettext (str)


#define CONVERT_STRING(string, lcontext) \
  string = from_current_source_encoding (string, lcontext, pos->file_name, \
                                         pos->line_number);


/* Update the is_format[] flags depending on the information given in the
   context.  */
static void
set_format_flags_from_context (enum is_format is_format[NFORMATS],
                               flag_context_ty context, const char *string,
                               lex_pos_ty *pos, const char *pretty_msgstr)
{
  size_t i;

  if (context.is_format1 != undecided
      || context.is_format2 != undecided
      || context.is_format3 != undecided
      || context.is_format4 != undecided)
    for (i = 0; i < NFORMATS; i++)
      {
        if (is_format[i] == undecided)
          {
            if (formatstring_parsers[i] == current_formatstring_parser1
                && context.is_format1 != undecided)
              is_format[i] = (enum is_format) context.is_format1;
            if (formatstring_parsers[i] == current_formatstring_parser2
                && context.is_format2 != undecided)
              is_format[i] = (enum is_format) context.is_format2;
            if (formatstring_parsers[i] == current_formatstring_parser3
                && context.is_format3 != undecided)
              is_format[i] = (enum is_format) context.is_format3;
            if (formatstring_parsers[i] == current_formatstring_parser4
                && context.is_format4 != undecided)
              is_format[i] = (enum is_format) context.is_format4;
          }
        if (possible_format_p (is_format[i]))
          {
            struct formatstring_parser *parser = formatstring_parsers[i];
            char *invalid_reason = NULL;
            void *descr = parser->parse (string, false, NULL, &invalid_reason);

            if (descr != NULL)
              parser->free (descr);
            else
              {
                /* The string is not a valid format string.  */
                if (is_format[i] != possible)
                  {
                    char buffer[22];

                    error_with_progname = false;
                    if (pos->line_number == (size_t)(-1))
                      buffer[0] = '\0';
                    else
                      sprintf (buffer, ":%ld", (long) pos->line_number);
                    multiline_warning (xasprintf (_("%s%s: warning: "),
                                                  pos->file_name, buffer),
                                       xasprintf (is_format[i] == yes_according_to_context
                                                  ? _("Although being used in a format string position, the %s is not a valid %s format string. Reason: %s\n")
                                                  : _("Although declared as such, the %s is not a valid %s format string. Reason: %s\n"),
                                                  pretty_msgstr,
                                                  format_language_pretty[i],
                                                  invalid_reason));
                    error_with_progname = true;
                  }

                is_format[i] = impossible;
                free (invalid_reason);
              }
          }
      }
}


void
decide_is_format (message_ty *mp)
{
  size_t i;

  /* If it is not already decided, through programmer comments, whether the
     msgid is a format string, examine the msgid.  This is a heuristic.  */
  for (i = 0; i < NFORMATS; i++)
    {
      if (mp->is_format[i] == undecided
          && (formatstring_parsers[i] == current_formatstring_parser1
              || formatstring_parsers[i] == current_formatstring_parser2
              || formatstring_parsers[i] == current_formatstring_parser3
              || formatstring_parsers[i] == current_formatstring_parser4)
          /* But avoid redundancy: objc-format is stronger than c-format.  */
          && !(i == format_c && possible_format_p (mp->is_format[format_objc]))
          && !(i == format_objc && possible_format_p (mp->is_format[format_c]))
          /* Avoid flagging a string as c-format when it's known to be a
             qt-format or qt-plural-format or kde-format or boost-format
             string.  */
          && !(i == format_c
               && (possible_format_p (mp->is_format[format_qt])
                   || possible_format_p (mp->is_format[format_qt_plural])
                   || possible_format_p (mp->is_format[format_kde])
                   || possible_format_p (mp->is_format[format_kde_kuit])
                   || possible_format_p (mp->is_format[format_boost])))
          /* Avoid flagging a string as kde-format when it's known to
             be a kde-kuit-format string.  */
          && !(i == format_kde
               && possible_format_p (mp->is_format[format_kde_kuit]))
          /* Avoid flagging a string as kde-kuit-format when it's
             known to be a kde-format string.  Note that this relies
             on the fact that format_kde < format_kde_kuit, so a
             string will be marked as kde-format if both are
             undecided.  */
          && !(i == format_kde_kuit
               && possible_format_p (mp->is_format[format_kde])))
        {
          struct formatstring_parser *parser = formatstring_parsers[i];
          char *invalid_reason = NULL;
          void *descr = parser->parse (mp->msgid, false, NULL, &invalid_reason);

          if (descr != NULL)
            {
              /* msgid is a valid format string.  We mark only those msgids
                 as format strings which contain at least one format directive
                 and thus are format strings with a high probability.  We
                 don't mark strings without directives as format strings,
                 because that would force the programmer to add
                 "xgettext: no-c-format" anywhere where a translator wishes
                 to use a percent sign.  So, the msgfmt checking will not be
                 perfect.  Oh well.  */
              if (parser->get_number_of_directives (descr) > 0
                  && !(parser->is_unlikely_intentional != NULL
                       && parser->is_unlikely_intentional (descr)))
                mp->is_format[i] = possible;

              parser->free (descr);
            }
          else
            {
              /* msgid is not a valid format string.  */
              mp->is_format[i] = impossible;
              free (invalid_reason);
            }
        }
    }
}

void
intersect_range (message_ty *mp, const struct argument_range *range)
{
  if (has_range_p (*range))
    {
      if (has_range_p (mp->range))
        {
          if (range->min < mp->range.min)
            mp->range.min = range->min;
          if (range->max > mp->range.max)
            mp->range.max = range->max;
        }
      else
        mp->range = *range;
    }
}

void
decide_do_wrap (message_ty *mp)
{
  /* By default we wrap.  */
  mp->do_wrap = (mp->do_wrap == no ? no : yes);
}

void
decide_syntax_check (message_ty *mp)
{
  size_t i;

  for (i = 0; i < NSYNTAXCHECKS; i++)
    if (mp->do_syntax_check[i] == undecided)
      mp->do_syntax_check[i] = default_syntax_check[i] == yes ? yes : no;
}


static void
warn_format_string (enum is_format is_format[NFORMATS], const char *string,
                    lex_pos_ty *pos, const char *pretty_msgstr)
{
  if (possible_format_p (is_format[format_python])
      && get_python_format_unnamed_arg_count (string) > 1)
    {
      char buffer[22];

      error_with_progname = false;
      if (pos->line_number == (size_t)(-1))
        buffer[0] = '\0';
      else
        sprintf (buffer, ":%ld", (long) pos->line_number);
      multiline_warning (xasprintf (_("%s%s: warning: "),
                                    pos->file_name, buffer),
                         xasprintf (_("\
'%s' format string with unnamed arguments cannot be properly localized:\n\
The translator cannot reorder the arguments.\n\
Please consider using a format string with named arguments,\n\
and a mapping instead of a tuple for the arguments.\n"),
                                    pretty_msgstr));
      error_with_progname = true;
    }
}


message_ty *
remember_a_message (message_list_ty *mlp, char *msgctxt, char *msgid,
                    bool is_utf8, bool pluralp, flag_context_ty context,
                    lex_pos_ty *pos,
                    const char *extracted_comment,
                    refcounted_string_list_ty *comment, bool comment_is_utf8)
{
  enum is_format is_format[NFORMATS];
  struct argument_range range;
  enum is_wrap do_wrap;
  enum is_syntax_check do_syntax_check[NSYNTAXCHECKS];
  message_ty *mp;
  size_t i;

  /* See whether we shall exclude this message.  */
  if (exclude != NULL && message_list_search (exclude, msgctxt, msgid) != NULL)
    {
      /* Tell the lexer to reset its comment buffer, so that the next
         message gets the correct comments.  */
      xgettext_comment_reset ();
      savable_comment_reset ();

      if (msgctxt != NULL)
        free (msgctxt);
      free (msgid);

      return NULL;
    }

  savable_comment_to_xgettext_comment (comment);

  for (i = 0; i < NFORMATS; i++)
    is_format[i] = undecided;
  range.min = -1;
  range.max = -1;
  do_wrap = undecided;
  for (i = 0; i < NSYNTAXCHECKS; i++)
    do_syntax_check[i] = undecided;

  if (!is_utf8)
    {
      if (msgctxt != NULL)
        CONVERT_STRING (msgctxt, lc_string);
      CONVERT_STRING (msgid, lc_string);
    }

  if (msgctxt == NULL && msgid[0] == '\0' && !xgettext_omit_header)
    {
      char buffer[22];

      error_with_progname = false;
      if (pos->line_number == (size_t)(-1))
        buffer[0] = '\0';
      else
        sprintf (buffer, ":%ld", (long) pos->line_number);
      multiline_warning (xasprintf (_("%s%s: warning: "), pos->file_name,
                                    buffer),
                         xstrdup (_("\
Empty msgid.  It is reserved by GNU gettext:\n\
gettext(\"\") returns the header entry with\n\
meta information, not the empty string.\n")));
      error_with_progname = true;
    }

  /* See if we have seen this message before.  */
  mp = message_list_search (mlp, msgctxt, msgid);
  if (mp != NULL)
    {
      if (pluralp != (mp->msgid_plural != NULL))
        {
          lex_pos_ty pos1;
          lex_pos_ty pos2;
          char buffer1[22];
          char buffer2[22];

          if (pluralp)
            {
              pos1 = mp->pos;
              pos2 = *pos;
            }
          else
            {
              pos1 = *pos;
              pos2 = mp->pos;
            }

          if (pos1.line_number == (size_t)(-1))
            buffer1[0] = '\0';
          else
            sprintf (buffer1, ":%ld", (long) pos1.line_number);
          if (pos2.line_number == (size_t)(-1))
            buffer2[0] = '\0';
          else
            sprintf (buffer2, ":%ld", (long) pos2.line_number);
          multiline_warning (xstrdup (_("warning: ")),
                             xasprintf ("%s\n%s\n%s\n%s\n",
                                        xasprintf (_("msgid '%s' is used without plural and with plural."),
                                                   msgid),
                                        xasprintf (_("%s%s: Here is the occurrence without plural."),
                                                   pos1.file_name, buffer1),
                                        xasprintf (_("%s%s: Here is the occurrence with plural."),
                                                   pos2.file_name, buffer2),
                                        xstrdup (_("Workaround: If the msgid is a sentence, change the wording of the sentence; otherwise, use contexts for disambiguation."))));
        }

      if (msgctxt != NULL)
        free (msgctxt);
      free (msgid);
      for (i = 0; i < NFORMATS; i++)
        is_format[i] = mp->is_format[i];
      do_wrap = mp->do_wrap;
      for (i = 0; i < NSYNTAXCHECKS; i++)
        do_syntax_check[i] = mp->do_syntax_check[i];
    }
  else
    {
      const char *msgstr;

      /* Construct the msgstr from the prefix and suffix, otherwise use the
         empty string.  */
      if (msgstr_prefix)
        {
          msgstr = xasprintf ("%s%s%s", msgstr_prefix, msgid, msgstr_suffix);
          assume (msgstr != NULL);
        }
      else
        msgstr = "";

      /* Allocate a new message and append the message to the list.  */
      mp = message_alloc (msgctxt, msgid, NULL, msgstr, strlen (msgstr) + 1,
                          pos);
      /* Do not free msgctxt and msgid.  */
      message_list_append (mlp, mp);
    }

  /* Determine whether the context specifies that the msgid is a format
     string.  */
  set_format_flags_from_context (is_format, context, mp->msgid, pos, "msgid");

  /* Ask the lexer for the comments it has seen.  */
  {
    size_t nitems_before;
    size_t nitems_after;
    int j;
    bool add_all_remaining_comments;
    /* The string before the comment tag.  For example, If "** TRANSLATORS:"
       is seen and the comment tag is "TRANSLATORS:",
       then comment_tag_prefix is set to "** ".  */
    const char *comment_tag_prefix = "";
    size_t comment_tag_prefix_length = 0;

    nitems_before = (mp->comment_dot != NULL ? mp->comment_dot->nitems : 0);

    if (extracted_comment != NULL)
      {
        char *copy = xstrdup (extracted_comment);
        char *rest;

        rest = copy;
        while (*rest != '\0')
          {
            char *newline = strchr (rest, '\n');

            if (newline != NULL)
              {
                *newline = '\0';
                message_comment_dot_append (mp, rest);
                rest = newline + 1;
              }
            else
              {
                message_comment_dot_append (mp, rest);
                break;
              }
          }
        free (copy);
      }

    add_all_remaining_comments = add_all_comments;
    for (j = 0; ; ++j)
      {
        const char *s = xgettext_comment (j);
        const char *t;
        if (s == NULL)
          break;

        if (!comment_is_utf8)
          CONVERT_STRING (s, lc_comment);

        /* To reduce the possibility of unwanted matches we do a two
           step match: the line must contain 'xgettext:' and one of
           the possible format description strings.  */
        if ((t = c_strstr (s, "xgettext:")) != NULL)
          {
            bool tmp_fuzzy;
            enum is_format tmp_format[NFORMATS];
            struct argument_range tmp_range;
            enum is_wrap tmp_wrap;
            enum is_syntax_check tmp_syntax_check[NSYNTAXCHECKS];
            bool interesting;

            t += strlen ("xgettext:");

            po_parse_comment_special (t, &tmp_fuzzy, tmp_format, &tmp_range,
                                      &tmp_wrap, tmp_syntax_check);

            interesting = false;
            for (i = 0; i < NFORMATS; i++)
              if (tmp_format[i] != undecided)
                {
                  is_format[i] = tmp_format[i];
                  interesting = true;
                }
            if (has_range_p (tmp_range))
              {
                range = tmp_range;
                interesting = true;
              }
            if (tmp_wrap != undecided)
              {
                do_wrap = tmp_wrap;
                interesting = true;
              }
            for (i = 0; i < NSYNTAXCHECKS; i++)
              if (tmp_syntax_check[i] != undecided)
                {
                  do_syntax_check[i] = tmp_syntax_check[i];
                  interesting = true;
                }

            /* If the "xgettext:" marker was followed by an interesting
               keyword, and we updated our is_format/do_wrap variables,
               we don't print the comment as a #. comment.  */
            if (interesting)
              continue;
          }

        if (!add_all_remaining_comments && comment_tag != NULL)
          {
            /* When the comment tag is seen, it drags in not only the line
               which it starts, but all remaining comment lines.  */
            if ((t = c_strstr (s, comment_tag)) != NULL)
              {
                add_all_remaining_comments = true;
                comment_tag_prefix = s;
                comment_tag_prefix_length = t - s;
              }
          }

        if (add_all_remaining_comments)
          {
            if (strncmp (s, comment_tag_prefix, comment_tag_prefix_length) == 0)
              s += comment_tag_prefix_length;
            message_comment_dot_append (mp, s);
          }
      }

    nitems_after = (mp->comment_dot != NULL ? mp->comment_dot->nitems : 0);

    /* Don't add the comments if they are a repetition of the tail of the
       already present comments.  This avoids unneeded duplication if the
       same message appears several times, each time with the same comment.  */
    if (nitems_before < nitems_after)
      {
        size_t added = nitems_after - nitems_before;

        if (added <= nitems_before)
          {
            bool repeated = true;

            for (i = 0; i < added; i++)
              if (strcmp (mp->comment_dot->item[nitems_before - added + i],
                          mp->comment_dot->item[nitems_before + i]) != 0)
                {
                  repeated = false;
                  break;
                }

            if (repeated)
              {
                for (i = 0; i < added; i++)
                  free ((char *) mp->comment_dot->item[nitems_before + i]);
                mp->comment_dot->nitems = nitems_before;
              }
          }
      }
  }

  for (i = 0; i < NFORMATS; i++)
    mp->is_format[i] = is_format[i];
  decide_is_format (mp);

  intersect_range (mp, &range);

  mp->do_wrap = do_wrap;
  decide_do_wrap (mp);

  for (i = 0; i < NSYNTAXCHECKS; i++)
    mp->do_syntax_check[i] = do_syntax_check[i];
  decide_syntax_check (mp);

  /* Warn about the use of non-reorderable format strings when the programming
     language also provides reorderable format strings.  */
  warn_format_string (is_format, mp->msgid, pos, "msgid");

  /* Remember where we saw this msgid.  */
  message_comment_filepos (mp, pos->file_name, pos->line_number);

  /* Tell the lexer to reset its comment buffer, so that the next
     message gets the correct comments.  */
  xgettext_comment_reset ();
  savable_comment_reset ();

  return mp;
}


void
remember_a_message_plural (message_ty *mp, char *string, bool is_utf8,
                           flag_context_ty context, lex_pos_ty *pos,
                           refcounted_string_list_ty *comment,
                           bool comment_is_utf8)
{
  char *msgid_plural;

  msgid_plural = string;

  savable_comment_to_xgettext_comment (comment);

  if (!is_utf8)
    CONVERT_STRING (msgid_plural, lc_string);

  /* See if the message is already a plural message.  */
  if (mp->msgid_plural == NULL)
    {
      char *msgstr1_malloc = NULL;
      const char *msgstr1;
      size_t msgstr1_len;
      char *msgstr;
      size_t i;

      mp->msgid_plural = msgid_plural;

      /* Construct the first plural form from the prefix and suffix,
         otherwise use the empty string.  The translator will have to
         provide additional plural forms.  */
      if (msgstr_prefix)
        {
          msgstr1_malloc =
            xasprintf ("%s%s%s", msgstr_prefix, msgid_plural, msgstr_suffix);
          msgstr1 = msgstr1_malloc;
          assume (msgstr1 != NULL);
        }
      else
        msgstr1 = "";
      msgstr1_len = strlen (msgstr1) + 1;
      msgstr = XNMALLOC (mp->msgstr_len + msgstr1_len, char);
      memcpy (msgstr, mp->msgstr, mp->msgstr_len);
      memcpy (msgstr + mp->msgstr_len, msgstr1, msgstr1_len);
      mp->msgstr = msgstr;
      mp->msgstr_len = mp->msgstr_len + msgstr1_len;
      free (msgstr1_malloc);

      /* Determine whether the context specifies that the msgid_plural is a
         format string.  */
      set_format_flags_from_context (mp->is_format, context, mp->msgid_plural,
                                     pos, "msgid_plural");

      /* If it is not already decided, through programmer comments or
         the msgid, whether the msgid is a format string, examine the
         msgid_plural.  This is a heuristic.  */
      for (i = 0; i < NFORMATS; i++)
        if ((formatstring_parsers[i] == current_formatstring_parser1
             || formatstring_parsers[i] == current_formatstring_parser2
             || formatstring_parsers[i] == current_formatstring_parser3
             || formatstring_parsers[i] == current_formatstring_parser4)
            && (mp->is_format[i] == undecided || mp->is_format[i] == possible)
            /* But avoid redundancy: objc-format is stronger than c-format.  */
            && !(i == format_c
                 && possible_format_p (mp->is_format[format_objc]))
            && !(i == format_objc
                 && possible_format_p (mp->is_format[format_c]))
            /* Avoid flagging a string as c-format when it's known to be a
               qt-format or qt-plural-format or boost-format string.  */
            && !(i == format_c
                 && (possible_format_p (mp->is_format[format_qt])
                     || possible_format_p (mp->is_format[format_qt_plural])
                     || possible_format_p (mp->is_format[format_kde])
                     || possible_format_p (mp->is_format[format_kde_kuit])
                     || possible_format_p (mp->is_format[format_boost])))
            /* Avoid flagging a string as kde-format when it's known
               to be a kde-kuit-format string.  */
            && !(i == format_kde
                 && possible_format_p (mp->is_format[format_kde_kuit]))
            /* Avoid flagging a string as kde-kuit-format when it's
               known to be a kde-format string.  Note that this relies
               on the fact that format_kde < format_kde_kuit, so a
               string will be marked as kde-format if both are
               undecided.  */
            && !(i == format_kde_kuit
                 && possible_format_p (mp->is_format[format_kde])))
          {
            struct formatstring_parser *parser = formatstring_parsers[i];
            char *invalid_reason = NULL;
            void *descr =
              parser->parse (mp->msgid_plural, false, NULL, &invalid_reason);

            if (descr != NULL)
              {
                /* Same heuristic as in remember_a_message.  */
                if (parser->get_number_of_directives (descr) > 0
                    && !(parser->is_unlikely_intentional != NULL
                         && parser->is_unlikely_intentional (descr)))
                  mp->is_format[i] = possible;

                parser->free (descr);
              }
            else
              {
                /* msgid_plural is not a valid format string.  */
                mp->is_format[i] = impossible;
                free (invalid_reason);
              }
          }

      /* Warn about the use of non-reorderable format strings when the programming
         language also provides reorderable format strings.  */
      warn_format_string (mp->is_format, mp->msgid_plural, pos, "msgid_plural");
    }
  else
    free (msgid_plural);

  /* Tell the lexer to reset its comment buffer, so that the next
     message gets the correct comments.  */
  xgettext_comment_reset ();
  savable_comment_reset ();
}
