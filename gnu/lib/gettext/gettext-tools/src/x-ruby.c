/* xgettext Ruby backend.
   Copyright (C) 2020 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2020.

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
#include "x-ruby.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"
#include "sh-quote.h"
#include "spawn-pipe.h"
#include "wait-process.h"
#include "xvasprintf.h"
#include "x-po.h"
#include "xgettext.h"
#include "xg-message.h"
#include "c-strstr.h"
#include "read-catalog-abstract.h"
#include "error.h"
#include "gettext.h"

/* A convenience macro.  I don't like writing gettext() every time.  */
#define _(str) gettext (str)

/* The Ruby syntax is defined in
   https://ruby-doc.org/core-2.7.1/doc/syntax_rdoc.html
   https://ruby-doc.org/core-2.7.1/doc/syntax/comments_rdoc.html
   https://ruby-doc.org/core-2.7.1/doc/syntax/literals_rdoc.html
   We don't parse Ruby directly, but instead rely on the 'rxgettext' program
   from https://github.com/ruby-gettext/gettext .  */


/* ====================== Keyword set customization.  ====================== */

/* This function currently has no effect.  */
void
x_ruby_extract_all (void)
{
}

/* This function currently has no effect.  */
void
x_ruby_keyword (const char *keyword)
{
}

/* This function currently has no effect.  */
void
init_flag_table_ruby (void)
{
}


/* ========================= Extracting strings.  ========================== */

void
extract_ruby (const char *found_in_dir, const char *real_filename,
              const char *logical_filename,
              flag_context_list_table_ty *flag_table,
              msgdomain_list_ty *mdlp)
{
  const char *progname = "rxgettext";
  char *dummy_filename;
  msgdomain_list_ty *mdlp2;
  int pass;

  dummy_filename = xasprintf (_("(output from '%s')"), progname);

  /* Invoke rgettext twice:
     1. to get the messages, without ruby-format flags.
     2. to get the 'xgettext:' comments that guide us while adding
        [no-]ruby-format flags.  */
  mdlp2 = msgdomain_list_alloc (true);
  for (pass = 0; pass < 2; pass++)
    {
      const char *argv[4];
      unsigned int i;
      pid_t child;
      int fd[1];
      FILE *fp;
      int exitstatus;

      /* Prepare arguments.  */
      argv[0] = progname;
      i = 1;

      if (pass > 0)
        argv[i++] = "--add-comments=xgettext:";
      else
        {
          if (add_all_comments)
            argv[i++] = "--add-comments";
          else if (comment_tag != NULL)
            argv[i++] = xasprintf ("--add-comments=%s", comment_tag);
        }

      argv[i++] = logical_filename;

      argv[i] = NULL;

      if (verbose)
        {
          char *command = shell_quote_argv (argv);
          error (0, 0, "%s", command);
          free (command);
        }

      child = create_pipe_in (progname, progname, argv, found_in_dir,
                              DEV_NULL, false, true, true, fd);

      fp = fdopen (fd[0], "r");
      if (fp == NULL)
        error (EXIT_FAILURE, errno, _("fdopen() failed"));

      /* Read the resulting PO file.  */
      extract_po (fp, dummy_filename, dummy_filename, flag_table,
                  pass == 0 ? mdlp : mdlp2);

      fclose (fp);

      /* Remove zombie process from process list, and retrieve exit status.  */
      exitstatus =
        wait_subprocess (child, progname, false, false, true, true, NULL);
      if (exitstatus != 0)
        error (EXIT_FAILURE, 0, _("%s subprocess failed with exit code %d"),
               progname, exitstatus);
    }

  /* Add [no-]ruby-format flags and process 'xgettext:' comments.
     This processing is similar to the one done in remember_a_message().  */
  if (mdlp->nitems == 1 && mdlp2->nitems == 1)
    {
      message_list_ty *mlp = mdlp->item[0]->messages;
      message_list_ty *mlp2 = mdlp2->item[0]->messages;
      size_t j;

      for (j = 0; j < mlp->nitems; j++)
        {
          message_ty *mp = mlp->item[j];

          if (!is_header (mp))
            {
              /* Find 'xgettext:' comments and apply them to mp.  */
              message_ty *mp2 =
                message_list_search (mlp2, mp->msgctxt, mp->msgid);

              if (mp2 != NULL && mp2->comment_dot != NULL)
                {
                  string_list_ty *mp2_comment_dot = mp2->comment_dot;
                  size_t k;

                  for (k = 0; k < mp2_comment_dot->nitems; k++)
                    {
                      const char *s = mp2_comment_dot->item[k];

                      /* To reduce the possibility of unwanted matches we do a
                         two step match: the line must contain 'xgettext:' and
                         one of the possible format description strings.  */
                      const char *t = c_strstr (s, "xgettext:");
                      if (t != NULL)
                        {
                          bool tmp_fuzzy;
                          enum is_format tmp_format[NFORMATS];
                          struct argument_range tmp_range;
                          enum is_wrap tmp_wrap;
                          enum is_syntax_check tmp_syntax_check[NSYNTAXCHECKS];
                          bool interesting;
                          size_t i;

                          t += strlen ("xgettext:");

                          po_parse_comment_special (t, &tmp_fuzzy, tmp_format,
                                                    &tmp_range, &tmp_wrap,
                                                    tmp_syntax_check);

                          interesting = false;
                          for (i = 0; i < NFORMATS; i++)
                            if (tmp_format[i] != undecided)
                              {
                                mp->is_format[i] = tmp_format[i];
                                interesting = true;
                              }
                          if (has_range_p (tmp_range))
                            {
                              intersect_range (mp, &tmp_range);
                              interesting = true;
                            }
                          if (tmp_wrap != undecided)
                            {
                              mp->do_wrap = tmp_wrap;
                              interesting = true;
                            }
                          for (i = 0; i < NSYNTAXCHECKS; i++)
                            if (tmp_syntax_check[i] != undecided)
                              {
                                mp->do_syntax_check[i] = tmp_syntax_check[i];
                                interesting = true;
                              }

                          /* If the "xgettext:" marker was followed by an
                             interesting keyword, and we updated our
                             is_format/do_wrap variables, eliminate the comment
                             from the #. comments.  */
                          if (interesting)
                            if (mp->comment_dot != NULL)
                              {
                                const char *removed =
                                  string_list_remove (mp->comment_dot, s);

                                if (removed != NULL)
                                  free ((char *) removed);
                              }
                        }
                    }
                }

              /* Now evaluate the consequences of the 'xgettext:' comments,  */
              decide_is_format (mp);
              decide_do_wrap (mp);
              decide_syntax_check (mp);
            }
        }
    }

  msgdomain_list_free (mdlp2);

  free (dummy_filename);
}
