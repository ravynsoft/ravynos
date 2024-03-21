/* Format strings.
   Copyright (C) 2001-2010, 2012-2013, 2015, 2019-2020, 2023 Free Software Foundation, Inc.
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
# include <config.h>
#endif

/* Specification.  */
#include "format.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "message.h"
#include "gettext.h"

#define _(str) gettext (str)

/* Table of all format string parsers.  */
struct formatstring_parser *formatstring_parsers[NFORMATS] =
{
  /* format_c */                &formatstring_c,
  /* format_objc */             &formatstring_objc,
  /* format_cplusplus_brace */  &formatstring_cplusplus_brace,
  /* format_python */           &formatstring_python,
  /* format_python_brace */     &formatstring_python_brace,
  /* format_java */             &formatstring_java,
  /* format_java_printf */      &formatstring_java_printf,
  /* format_csharp */           &formatstring_csharp,
  /* format_javascript */       &formatstring_javascript,
  /* format_scheme */           &formatstring_scheme,
  /* format_lisp */             &formatstring_lisp,
  /* format_elisp */            &formatstring_elisp,
  /* format_librep */           &formatstring_librep,
  /* format_ruby */             &formatstring_ruby,
  /* format_sh */               &formatstring_sh,
  /* format_awk */              &formatstring_awk,
  /* format_lua */              &formatstring_lua,
  /* format_pascal */           &formatstring_pascal,
  /* format_smalltalk */        &formatstring_smalltalk,
  /* format_qt */               &formatstring_qt,
  /* format_qt_plural */        &formatstring_qt_plural,
  /* format_kde */              &formatstring_kde,
  /* format_kde_kuit */         &formatstring_kde_kuit,
  /* format_boost */            &formatstring_boost,
  /* format_tcl */              &formatstring_tcl,
  /* format_perl */             &formatstring_perl,
  /* format_perl_brace */       &formatstring_perl_brace,
  /* format_php */              &formatstring_php,
  /* format_gcc_internal */     &formatstring_gcc_internal,
  /* format_gfc_internal */     &formatstring_gfc_internal,
  /* format_ycp */              &formatstring_ycp
};

/* Check whether both formats strings contain compatible format
   specifications for format type i (0 <= i < NFORMATS).  */
int
check_msgid_msgstr_format_i (const char *msgid, const char *msgid_plural,
                             const char *msgstr, size_t msgstr_len,
                             size_t i,
                             struct argument_range range,
                             const struct plural_distribution *distribution,
                             formatstring_error_logger_t error_logger)
{
  int seen_errors = 0;

  /* At runtime, we can assume the program passes arguments that fit well for
     msgid.  We must signal an error if msgstr wants more arguments that msgid
     accepts.
     If msgstr wants fewer arguments than msgid, it wouldn't lead to a crash
     at runtime, but we nevertheless give an error because
     1) this situation occurs typically after the programmer has added some
        arguments to msgid, so we must make the translator specially aware
        of it (more than just "fuzzy"),
     2) it is generally wrong if a translation wants to ignore arguments that
        are used by other translations.  */

  struct formatstring_parser *parser = formatstring_parsers[i];
  char *invalid_reason = NULL;
  void *msgid_descr =
    parser->parse (msgid_plural != NULL ? msgid_plural : msgid, false, NULL,
                   &invalid_reason);

  if (msgid_descr != NULL)
    {
      const char *pretty_msgid =
        (msgid_plural != NULL ? "msgid_plural" : "msgid");
      char buf[18+1];
      const char *pretty_msgstr = "msgstr";
      bool has_plural_translations = (strlen (msgstr) + 1 < msgstr_len);
      const char *p_end = msgstr + msgstr_len;
      const char *p;
      unsigned int j;

      for (p = msgstr, j = 0; p < p_end; p += strlen (p) + 1, j++)
        {
          void *msgstr_descr;

          if (msgid_plural != NULL)
            {
              sprintf (buf, "msgstr[%u]", j);
              pretty_msgstr = buf;
            }

          msgstr_descr = parser->parse (p, true, NULL, &invalid_reason);

          if (msgstr_descr != NULL)
            {
              /* Use strict checking (require same number of format
                 directives on both sides) if the message has no plurals,
                 or if msgid_plural exists but on the msgstr[] side
                 there is only msgstr[0], or if distribution->often[j]
                 indicates that the variant applies to infinitely many
                 values of N and the N range is not restricted in a way
                 that the variant applies to only one N.
                 Use relaxed checking when there are at least two
                 msgstr[] forms and the distribution does not give more
                 precise information.  */
              bool strict_checking =
                (msgid_plural == NULL
                 || !has_plural_translations
                 || (distribution != NULL
                     && distribution->often != NULL
                     && j < distribution->often_length
                     && distribution->often[j]
                     && !(has_range_p (range)
                          && distribution->histogram (distribution,
                                                      range.min, range.max, j)
                             <= 1)));

              if (parser->check (msgid_descr, msgstr_descr,
                                 strict_checking,
                                 error_logger, pretty_msgid, pretty_msgstr))
                seen_errors++;

              parser->free (msgstr_descr);
            }
          else
            {
              error_logger (_("'%s' is not a valid %s format string, unlike '%s'. Reason: %s"),
                            pretty_msgstr, format_language_pretty[i],
                            pretty_msgid, invalid_reason);
              seen_errors++;
              free (invalid_reason);
            }
        }

      parser->free (msgid_descr);
    }
  else
    free (invalid_reason);

  return seen_errors;
}

/* Check whether both formats strings contain compatible format
   specifications.
   Return the number of errors that were seen.  */
int
check_msgid_msgstr_format (const char *msgid, const char *msgid_plural,
                           const char *msgstr, size_t msgstr_len,
                           const enum is_format is_format[NFORMATS],
                           struct argument_range range,
                           const struct plural_distribution *distribution,
                           formatstring_error_logger_t error_logger)
{
  int seen_errors = 0;
  size_t i;

  /* We check only those messages for which the msgid's is_format flag
     is one of 'yes' or 'possible'.  We don't check msgids with is_format
     'no' or 'impossible', to obey the programmer's order.  We don't check
     msgids with is_format 'undecided' because that would introduce too
     many checks, thus forcing the programmer to add "xgettext: no-c-format"
     anywhere where a translator wishes to use a percent sign.  */
  for (i = 0; i < NFORMATS; i++)
    if (possible_format_p (is_format[i]))
      seen_errors += check_msgid_msgstr_format_i (msgid, msgid_plural,
                                                  msgstr, msgstr_len, i,
                                                  range,
                                                  distribution,
                                                  error_logger);

  return seen_errors;
}
