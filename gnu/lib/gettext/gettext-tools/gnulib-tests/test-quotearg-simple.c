/* Test of quotearg family of functions.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <https://www.gnu.org/licenses/>.  */

/* Written by Eric Blake <ebb9@byu.net>, 2008.  */

#include <config.h>

#include "quotearg.h"

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "localcharset.h"
#include "macros.h"
#include "zerosize-ptr.h"

#include "test-quotearg.h"

static struct result_groups results_g[] = {
  /* literal_quoting_style */
  { { "", "\0""1\0", 3, "simple", " \t\n'\"\033?""?/\\", "a:b", "a\\b",
      "a' b", LQ RQ, LQ RQ },
    { "", "1", 1, "simple", " \t\n'\"\033?""?/\\", "a:b", "a\\b",
      "a' b", LQ RQ, LQ RQ },
    { "", "1", 1, "simple", " \t\n'\"\033?""?/\\", "a:b", "a\\b",
      "a' b", LQ RQ, LQ RQ } },

  /* shell_quoting_style */
  { { "''", "\0""1\0", 3, "simple", "' \t\n'\\''\"\033?""?/\\'", "a:b",
      "'a\\b'", "\"a' b\"", LQ RQ, LQ RQ },
    { "''", "1", 1, "simple", "' \t\n'\\''\"\033?""?/\\'", "a:b",
      "'a\\b'", "\"a' b\"", LQ RQ, LQ RQ },
    { "''", "1", 1, "simple", "' \t\n'\\''\"\033?""?/\\'", "'a:b'",
      "'a\\b'", "\"a' b\"", LQ RQ, LQ RQ } },

  /* shell_always_quoting_style */
  { { "''", "'\0""1\0'", 5, "'simple'", "' \t\n'\\''\"\033?""?/\\'", "'a:b'",
      "'a\\b'", "\"a' b\"", "'" LQ RQ "'", "'" LQ RQ "'" },
    { "''", "'1'", 3, "'simple'", "' \t\n'\\''\"\033?""?/\\'", "'a:b'",
      "'a\\b'", "\"a' b\"", "'" LQ RQ "'", "'" LQ RQ "'" },
    { "''", "'1'", 3, "'simple'", "' \t\n'\\''\"\033?""?/\\'", "'a:b'",
      "'a\\b'", "\"a' b\"", "'" LQ RQ "'", "'" LQ RQ "'" } },

  /* shell_escape_quoting_style */
  { { "''", "''$'\\0''1'$'\\0'", 15, "simple",
      "' '$'\\t\\n'\\''\"'$'\\033''?""?/\\'", "a:b",
      "'a\\b'", "\"a' b\"", "''$'" LQ_ENC RQ_ENC "'", LQ RQ },
    { "''", "''$'\\0''1'$'\\0'", 15, "simple",
      "' '$'\\t\\n'\\''\"'$'\\033''?""?/\\'", "a:b",
      "'a\\b'", "\"a' b\"", "''$'" LQ_ENC RQ_ENC "'", LQ RQ },
    { "''", "''$'\\0''1'$'\\0'", 15, "simple",
      "' '$'\\t\\n'\\''\"'$'\\033''?""?/\\'", "'a:b'",
      "'a\\b'", "\"a' b\"", "''$'" LQ_ENC RQ_ENC "'", LQ RQ } },

  /* shell_escape_always_quoting_style */
  { { "''", "''$'\\0''1'$'\\0'", 15, "'simple'",
      "' '$'\\t\\n'\\''\"'$'\\033''?""?/\\'", "'a:b'",
      "'a\\b'", "\"a' b\"", "''$'" LQ_ENC RQ_ENC "'", "'" LQ RQ "'" },
    { "''", "''$'\\0''1'$'\\0'", 15, "'simple'",
      "' '$'\\t\\n'\\''\"'$'\\033''?""?/\\'", "'a:b'",
      "'a\\b'", "\"a' b\"", "''$'" LQ_ENC RQ_ENC "'", "'" LQ RQ "'" },
    { "''", "''$'\\0''1'$'\\0'", 15, "'simple'",
      "' '$'\\t\\n'\\''\"'$'\\033''?""?/\\'", "'a:b'",
      "'a\\b'", "\"a' b\"", "''$'" LQ_ENC RQ_ENC "'", "'" LQ RQ "'" } },

  /* c_quoting_style */
  { { "\"\"", "\"\\0001\\0\"", 9, "\"simple\"",
      "\" \\t\\n'\\\"\\033?""?/\\\\\"", "\"a:b\"", "\"a\\\\b\"",
      "\"a' b\"", "\"" LQ_ENC RQ_ENC "\"", "\"" LQ RQ "\"" },
    { "\"\"", "\"\\0001\\0\"", 9, "\"simple\"",
      "\" \\t\\n'\\\"\\033?""?/\\\\\"", "\"a:b\"", "\"a\\\\b\"",
      "\"a' b\"", "\"" LQ_ENC RQ_ENC "\"", "\"" LQ RQ "\"" },
    { "\"\"", "\"\\0001\\0\"", 9, "\"simple\"",
      "\" \\t\\n'\\\"\\033?""?/\\\\\"", "\"a\\:b\"", "\"a\\\\b\"",
      "\"a' b\"", "\"" LQ_ENC RQ_ENC "\"", "\"" LQ RQ "\"" } },

  /* c_maybe_quoting_style */
  { { "", "\"\\0001\\0\"", 9, "simple", "\" \\t\\n'\\\"\\033?""?/\\\\\"",
      "a:b", "a\\b", "a' b", "\"" LQ_ENC RQ_ENC "\"", LQ RQ },
    { "", "\"\\0001\\0\"", 9, "simple", "\" \\t\\n'\\\"\\033?""?/\\\\\"",
      "a:b", "a\\b", "a' b", "\"" LQ_ENC RQ_ENC "\"", LQ RQ },
    { "", "\"\\0001\\0\"", 9, "simple", "\" \\t\\n'\\\"\\033?""?/\\\\\"",
      "\"a:b\"", "a\\b", "a' b", "\"" LQ_ENC RQ_ENC "\"", LQ RQ } },

  /* escape_quoting_style */
  { { "", "\\0001\\0", 7, "simple", " \\t\\n'\"\\033?""?/\\\\", "a:b",
      "a\\\\b", "a' b", LQ_ENC RQ_ENC, LQ RQ },
    { "", "\\0001\\0", 7, "simple", " \\t\\n'\"\\033?""?/\\\\", "a:b",
      "a\\\\b", "a' b", LQ_ENC RQ_ENC, LQ RQ },
    { "", "\\0001\\0", 7, "simple", " \\t\\n'\"\\033?""?/\\\\", "a\\:b",
      "a\\\\b", "a' b", LQ_ENC RQ_ENC, LQ RQ } },

  /* locale_quoting_style */
  { { "''", "'\\0001\\0'", 9, "'simple'", "' \\t\\n\\'\"\\033?""?/\\\\'",
      "'a:b'", "'a\\\\b'", "'a\\' b'", "'" LQ_ENC RQ_ENC "'", "'" LQ RQ "'" },
    { "''", "'\\0001\\0'", 9, "'simple'", "' \\t\\n\\'\"\\033?""?/\\\\'",
      "'a:b'", "'a\\\\b'", "'a\\' b'", "'" LQ_ENC RQ_ENC "'", "'" LQ RQ "'" },
    { "''", "'\\0001\\0'", 9, "'simple'", "' \\t\\n\\'\"\\033?""?/\\\\'",
      "'a\\:b'", "'a\\\\b'", "'a\\' b'",
      "'" LQ_ENC RQ_ENC "'", "'" LQ RQ "'" } },

  /* clocale_quoting_style */
  { { "\"\"", "\"\\0001\\0\"", 9, "\"simple\"",
      "\" \\t\\n'\\\"\\033?""?/\\\\\"", "\"a:b\"", "\"a\\\\b\"",
      "\"a' b\"", "\"" LQ_ENC RQ_ENC "\"", "\"" LQ RQ "\"" },
    { "\"\"", "\"\\0001\\0\"", 9, "\"simple\"",
      "\" \\t\\n'\\\"\\033?""?/\\\\\"", "\"a:b\"", "\"a\\\\b\"",
      "\"a' b\"", "\"" LQ_ENC RQ_ENC "\"", "\"" LQ RQ "\"" },
    { "\"\"", "\"\\0001\\0\"", 9, "\"simple\"",
      "\" \\t\\n'\\\"\\033?""?/\\\\\"", "\"a\\:b\"", "\"a\\\\b\"",
      "\"a' b\"", "\"" LQ_ENC RQ_ENC "\"", "\"" LQ RQ "\"" } }
};

static struct result_groups flag_results[] = {
  /* literal_quoting_style and QA_ELIDE_NULL_BYTES */
  { { "", "1", 1, "simple", " \t\n'\"\033?""?/\\", "a:b", "a\\b", "a' b",
      LQ RQ, LQ RQ },
    { "", "1", 1, "simple", " \t\n'\"\033?""?/\\", "a:b", "a\\b", "a' b",
      LQ RQ, LQ RQ },
    { "", "1", 1, "simple", " \t\n'\"\033?""?/\\", "a:b", "a\\b", "a' b",
      LQ RQ, LQ RQ } },

  /* c_quoting_style and QA_ELIDE_OUTER_QUOTES */
  { { "", "\"\\0001\\0\"", 9, "simple", "\" \\t\\n'\\\"\\033?""?/\\\\\"",
      "a:b", "a\\b", "a' b", "\"" LQ_ENC RQ_ENC "\"", LQ RQ },
    { "", "\"\\0001\\0\"", 9, "simple", "\" \\t\\n'\\\"\\033?""?/\\\\\"",
      "a:b", "a\\b", "a' b", "\"" LQ_ENC RQ_ENC "\"", LQ RQ },
    { "", "\"\\0001\\0\"", 9, "simple", "\" \\t\\n'\\\"\\033?""?/\\\\\"",
      "\"a:b\"", "a\\b", "a' b", "\"" LQ_ENC RQ_ENC "\"", LQ RQ } },

  /* c_quoting_style and QA_SPLIT_TRIGRAPHS */
  { { "\"\"", "\"\\0001\\0\"", 9, "\"simple\"",
      "\" \\t\\n'\\\"\\033?\"\"?/\\\\\"", "\"a:b\"", "\"a\\\\b\"",
      "\"a' b\"", "\"" LQ_ENC RQ_ENC "\"", "\"" LQ RQ "\"" },
    { "\"\"", "\"\\0001\\0\"", 9, "\"simple\"",
      "\" \\t\\n'\\\"\\033?\"\"?/\\\\\"", "\"a:b\"", "\"a\\\\b\"",
      "\"a' b\"", "\"" LQ_ENC RQ_ENC "\"", "\"" LQ RQ "\"" },
    { "\"\"", "\"\\0001\\0\"", 9, "\"simple\"",
      "\" \\t\\n'\\\"\\033?\"\"?/\\\\\"", "\"a\\:b\"", "\"a\\\\b\"",
      "\"a' b\"", "\"" LQ_ENC RQ_ENC "\"", "\"" LQ RQ "\"" } }
};

static char const *custom_quotes[][2] = {
  { "", ""  },
  { "'", "'"  },
  { "(", ")"  },
  { ":", " "  },
  { " ", ":"  },
  { "# ", "\n" },
  { "\"'", "'\"" }
};

static struct result_groups custom_results[] = {
  /* left_quote = right_quote = "" */
  { { "", "\\0001\\0", 7, "simple",
      " \\t\\n'\"\\033?""?/\\\\", "a:b", "a\\\\b",
      "a' b", LQ_ENC RQ_ENC, LQ RQ },
    { "", "\\0001\\0", 7, "simple",
      " \\t\\n'\"\\033?""?/\\\\", "a:b", "a\\\\b",
      "a' b", LQ_ENC RQ_ENC, LQ RQ },
    { "", "\\0001\\0", 7, "simple",
      " \\t\\n'\"\\033?""?/\\\\", "a\\:b", "a\\\\b",
      "a' b", LQ_ENC RQ_ENC, LQ RQ } },

  /* left_quote = right_quote = "'" */
  { { "''", "'\\0001\\0'", 9, "'simple'",
      "' \\t\\n\\'\"\\033?""?/\\\\'", "'a:b'", "'a\\\\b'",
      "'a\\' b'", "'" LQ_ENC RQ_ENC "'", "'" LQ RQ "'" },
    { "''", "'\\0001\\0'", 9, "'simple'",
      "' \\t\\n\\'\"\\033?""?/\\\\'", "'a:b'", "'a\\\\b'",
      "'a\\' b'", "'" LQ_ENC RQ_ENC "'", "'" LQ RQ "'" },
    { "''", "'\\0001\\0'", 9, "'simple'",
      "' \\t\\n\\'\"\\033?""?/\\\\'", "'a\\:b'", "'a\\\\b'",
      "'a\\' b'", "'" LQ_ENC RQ_ENC "'", "'" LQ RQ "'" } },

  /* left_quote = "(" and right_quote = ")" */
  { { "()", "(\\0001\\0)", 9, "(simple)",
      "( \\t\\n'\"\\033?""?/\\\\)", "(a:b)", "(a\\\\b)",
      "(a' b)", "(" LQ_ENC RQ_ENC ")", "(" LQ RQ ")" },
    { "()", "(\\0001\\0)", 9, "(simple)",
      "( \\t\\n'\"\\033?""?/\\\\)", "(a:b)", "(a\\\\b)",
      "(a' b)", "(" LQ_ENC RQ_ENC ")", "(" LQ RQ ")" },
    { "()", "(\\0001\\0)", 9, "(simple)",
      "( \\t\\n'\"\\033?""?/\\\\)", "(a\\:b)", "(a\\\\b)",
      "(a' b)", "(" LQ_ENC RQ_ENC ")", "(" LQ RQ ")" } },

  /* left_quote = ":" and right_quote = " " */
  { { ": ", ":\\0001\\0 ", 9, ":simple ",
      ":\\ \\t\\n'\"\\033?""?/\\\\ ", ":a:b ", ":a\\\\b ",
      ":a'\\ b ", ":" LQ_ENC RQ_ENC " ", ":" LQ RQ " " },
    { ": ", ":\\0001\\0 ", 9, ":simple ",
      ":\\ \\t\\n'\"\\033?""?/\\\\ ", ":a:b ", ":a\\\\b ",
      ":a'\\ b ", ":" LQ_ENC RQ_ENC " ", ":" LQ RQ " " },
    { ": ", ":\\0001\\0 ", 9, ":simple ",
      ":\\ \\t\\n'\"\\033?""?/\\\\ ", ":a\\:b ", ":a\\\\b ",
      ":a'\\ b ", ":" LQ_ENC RQ_ENC " ", ":" LQ RQ " " } },

  /* left_quote = " " and right_quote = ":" */
  { { " :", " \\0001\\0:", 9, " simple:",
      "  \\t\\n'\"\\033?""?/\\\\:", " a\\:b:", " a\\\\b:",
      " a' b:", " " LQ_ENC RQ_ENC ":", " " LQ RQ ":" },
    { " :", " \\0001\\0:", 9, " simple:",
      "  \\t\\n'\"\\033?""?/\\\\:", " a\\:b:", " a\\\\b:",
      " a' b:", " " LQ_ENC RQ_ENC ":", " " LQ RQ ":" },
    { " :", " \\0001\\0:", 9, " simple:",
      "  \\t\\n'\"\\033?""?/\\\\:", " a\\:b:", " a\\\\b:",
      " a' b:", " " LQ_ENC RQ_ENC ":", " " LQ RQ ":" } },

  /* left_quote = "# " and right_quote = "\n" */
  { { "# \n", "# \\0001\\0\n", 10, "# simple\n",
      "#  \\t\\n'\"\\033?""?/\\\\\n", "# a:b\n", "# a\\\\b\n",
      "# a' b\n", "# " LQ_ENC RQ_ENC "\n", "# " LQ RQ "\n" },
    { "# \n", "# \\0001\\0\n", 10, "# simple\n",
      "#  \\t\\n'\"\\033?""?/\\\\\n", "# a:b\n", "# a\\\\b\n",
      "# a' b\n", "# " LQ_ENC RQ_ENC "\n", "# " LQ RQ "\n" },
    { "# \n", "# \\0001\\0\n", 10, "# simple\n",
      "#  \\t\\n'\"\\033?""?/\\\\\n", "# a\\:b\n", "# a\\\\b\n",
      "# a' b\n", "# " LQ_ENC RQ_ENC "\n", "# " LQ RQ "\n" } },

  /* left_quote = "\"'" and right_quote = "'\"" */
  { { "\"''\"", "\"'\\0001\\0'\"", 11, "\"'simple'\"",
      "\"' \\t\\n\\'\"\\033?""?/\\\\'\"", "\"'a:b'\"", "\"'a\\\\b'\"",
      "\"'a' b'\"", "\"'" LQ_ENC RQ_ENC "'\"", "\"'" LQ RQ "'\"" },
    { "\"''\"", "\"'\\0001\\0'\"", 11, "\"'simple'\"",
      "\"' \\t\\n\\'\"\\033?""?/\\\\'\"", "\"'a:b'\"", "\"'a\\\\b'\"",
      "\"'a' b'\"", "\"'" LQ_ENC RQ_ENC "'\"", "\"'" LQ RQ "'\"" },
    { "\"''\"", "\"'\\0001\\0'\"", 11, "\"'simple'\"",
      "\"' \\t\\n\\'\"\\033?""?/\\\\'\"", "\"'a\\:b'\"", "\"'a\\\\b'\"",
      "\"'a' b'\"", "\"'" LQ_ENC RQ_ENC "'\"", "\"'" LQ RQ "'\"" } }
};

static char *
use_quote_double_quotes (const char *str, size_t *len)
{
  char *p = *len == SIZE_MAX ? quotearg_char (str, '"')
                               : quotearg_char_mem (str, *len, '"');
  *len = strlen (p);
  return p;
}

int
main (_GL_UNUSED int argc, char *argv[])
{
  int i;
  bool ascii_only = MB_CUR_MAX == 1 && !isprint ((unsigned char) LQ[0]);

  /* This part of the program is hard-wired to the C locale since it
     does not call setlocale.  However, according to POSIX, the use of
     8-bit bytes in a character context in the C locale gives
     unspecified results (that is, the C locale charset is allowed to
     be unibyte with 8-bit bytes rejected [ASCII], unibyte with 8-bit
     bytes being characters [often ISO-8859-1], or multibyte [often
     UTF-8]).  We assume that the latter two cases will be
     indistinguishable in this test - that is, the LQ and RQ sequences
     will pass through unchanged in either type of charset.  So when
     testing for quoting of str7, use the ascii_only flag to decide
     what to expect for the 8-bit data being quoted.  */
  ASSERT (!isprint ('\033'));
  for (i = literal_quoting_style; i <= clocale_quoting_style; i++)
    {
      set_quoting_style (NULL, (enum quoting_style) i);
      if (!(i == locale_quoting_style || i == clocale_quoting_style)
          || (strcmp (locale_charset (), "ASCII") == 0
              || strcmp (locale_charset (), "ANSI_X3.4-1968") == 0))
        {
          compare_strings (use_quotearg_buffer, &results_g[i].group1,
                           ascii_only);
          compare_strings (use_quotearg, &results_g[i].group2,
                           ascii_only);
          if (i == c_quoting_style)
            compare_strings (use_quote_double_quotes, &results_g[i].group2,
                             ascii_only);
          compare_strings (use_quotearg_colon, &results_g[i].group3,
                           ascii_only);
        }
    }

  set_quoting_style (NULL, literal_quoting_style);
  ASSERT (set_quoting_flags (NULL, QA_ELIDE_NULL_BYTES) == 0);
  compare_strings (use_quotearg_buffer, &flag_results[0].group1, ascii_only);
  compare_strings (use_quotearg, &flag_results[0].group2, ascii_only);
  compare_strings (use_quotearg_colon, &flag_results[0].group3, ascii_only);

  set_quoting_style (NULL, c_quoting_style);
  ASSERT (set_quoting_flags (NULL, QA_ELIDE_OUTER_QUOTES)
          == QA_ELIDE_NULL_BYTES);
  compare_strings (use_quotearg_buffer, &flag_results[1].group1, ascii_only);
  compare_strings (use_quotearg, &flag_results[1].group2, ascii_only);
  compare_strings (use_quote_double_quotes, &flag_results[1].group2,
                   ascii_only);
  compare_strings (use_quotearg_colon, &flag_results[1].group3, ascii_only);

  ASSERT (set_quoting_flags (NULL, QA_SPLIT_TRIGRAPHS)
          == QA_ELIDE_OUTER_QUOTES);
  compare_strings (use_quotearg_buffer, &flag_results[2].group1, ascii_only);
  compare_strings (use_quotearg, &flag_results[2].group2, ascii_only);
  compare_strings (use_quote_double_quotes, &flag_results[2].group2,
                   ascii_only);
  compare_strings (use_quotearg_colon, &flag_results[2].group3, ascii_only);

  ASSERT (set_quoting_flags (NULL, 0) == QA_SPLIT_TRIGRAPHS);

  for (i = 0; i < sizeof custom_quotes / sizeof *custom_quotes; ++i)
    {
      set_custom_quoting (NULL,
                          custom_quotes[i][0], custom_quotes[i][1]);
      compare_strings (use_quotearg_buffer, &custom_results[i].group1,
                       ascii_only);
      compare_strings (use_quotearg, &custom_results[i].group2, ascii_only);
      compare_strings (use_quotearg_colon, &custom_results[i].group3,
                       ascii_only);
    }

  {
    /* Trigger the bug whereby quotearg_buffer would read beyond the NUL
       that defines the end of the string being quoted.  Use an input
       string whose NUL is the last byte before an unreadable page.  */
    char *z = zerosize_ptr ();

    if (z)
      {
        size_t q_len = 1024;
        char *q = malloc (q_len + 1);
        char buf[10];
        memset (q, 'Q', q_len);
        q[q_len] = 0;

        /* Z points to the boundary between a readable/writable page
           and one that is neither readable nor writable.  Position
           our string so its NUL is at the end of the writable one.  */
        char const *str = "____";
        size_t s_len = strlen (str);
        z -= s_len + 1;
        memcpy (z, str, s_len + 1);

        set_custom_quoting (NULL, q, q);
        /* Whether this actually triggers a SEGV depends on the
           implementation of memcmp: whether it compares only byte-at-
           a-time, and from left to right (no SEGV) or some other way.  */
        size_t n = quotearg_buffer (buf, sizeof buf, z, SIZE_MAX, NULL);
        ASSERT (n == s_len + 2 * q_len);
        ASSERT (memcmp (buf, q, sizeof buf) == 0);
        free (q);
      }
  }

  quotearg_free ();

  return 0;
}
