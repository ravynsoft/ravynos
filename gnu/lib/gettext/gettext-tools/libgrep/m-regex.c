/* Pattern Matchers for Regular Expressions.
   Copyright (C) 1992, 1998, 2000, 2005-2006, 2010, 2013 Free Software
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
#include "libgrep.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include "error.h"
#include "exitfail.h"
#include "xalloc.h"

#if defined (STDC_HEADERS) || (!defined (isascii) && !defined (HAVE_ISASCII))
# define IN_CTYPE_DOMAIN(c) 1
#else
# define IN_CTYPE_DOMAIN(c) isascii(c)
#endif
#define ISALNUM(C) (IN_CTYPE_DOMAIN (C) && isalnum (C))
#define IS_WORD_CONSTITUENT(C) (ISALNUM(C) || (C) == '_')

struct patterns
{
  /* Regex compiled regexp. */
  struct re_pattern_buffer regexbuf;
  struct re_registers regs; /* This is here on account of a BRAIN-DEAD
                               Q@#%!# library interface in regex.c.  */
};

struct compiled_regex {
  bool match_words;
  bool match_lines;
  char eolbyte;

  /* The Regex compiled patterns.  */
  struct patterns *patterns;
  size_t pcount;
};

static void *
compile (const char *pattern, size_t pattern_size,
         bool match_icase, bool match_words, bool match_lines, char eolbyte,
         reg_syntax_t syntax)
{
  struct compiled_regex *cregex;

  cregex = XMALLOC (struct compiled_regex);
  memset (cregex, '\0', sizeof (struct compiled_regex));
  cregex->match_words = match_words;
  cregex->match_lines = match_lines;
  cregex->eolbyte = eolbyte;
  cregex->patterns = NULL;
  cregex->pcount = 0;

  re_set_syntax (syntax);

  /* For GNU regex compiler we have to pass the patterns separately to detect
     errors like "[\nallo\n]\n".  The patterns here are "[", "allo" and "]"
     GNU regex should have raised a syntax error.  The same for backref, where
     the backref should have been local to each pattern.  */
  {
    const char *sep;
    size_t total = pattern_size;
    const char *motif = pattern;

    do
      {
        size_t len;
        const char *err;

        sep = (const char *) memchr (motif, '\n', total);
        if (sep)
          {
            len = sep - motif;
            sep++;
            total -= (len + 1);
          }
        else
          {
            len = total;
            total = 0;
          }

        cregex->patterns = xrealloc (cregex->patterns, (cregex->pcount + 1) * sizeof (struct patterns));
        memset (&cregex->patterns[cregex->pcount], '\0', sizeof (struct patterns));

        if ((err = re_compile_pattern (motif, len,
                                       &cregex->patterns[cregex->pcount].regexbuf)) != NULL)
          error (exit_failure, 0, "%s", err);
        cregex->pcount++;

        motif = sep;
      }
    while (sep && total != 0);
  }

  return cregex;
}

static void *
Gcompile (const char *pattern, size_t pattern_size,
          bool match_icase, bool match_words, bool match_lines, char eolbyte)
{
  return compile (pattern, pattern_size,
                  match_icase, match_words, match_lines, eolbyte,
                  RE_SYNTAX_GREP | RE_HAT_LISTS_NOT_NEWLINE);
}

static void *
Ecompile (const char *pattern, size_t pattern_size,
          bool match_icase, bool match_words, bool match_lines, char eolbyte)
{
  return compile (pattern, pattern_size,
                  match_icase, match_words, match_lines, eolbyte,
                  RE_SYNTAX_POSIX_EGREP);
}

static void *
AWKcompile (const char *pattern, size_t pattern_size,
            bool match_icase, bool match_words, bool match_lines, char eolbyte)
{
  return compile (pattern, pattern_size,
                  match_icase, match_words, match_lines, eolbyte,
                  RE_SYNTAX_AWK);
}

static size_t
EGexecute (const void *compiled_pattern,
           const char *buf, size_t buf_size,
           size_t *match_size, bool exact)
{
  struct compiled_regex *cregex = (struct compiled_regex *) compiled_pattern;
  char eol = cregex->eolbyte;
  register const char *buflim = buf + buf_size;
  register const char *beg;
  register const char *end;

  for (beg = buf; beg < buflim; beg = end)
    {
      size_t i;

      end = (const char *) memchr (beg, eol, buflim - beg);
      if (end == NULL)
        end = buflim;
      /* Here, either end < buflim && *end == eol, or end == buflim.  */

      for (i = 0; i < cregex->pcount; i++)
        {
          int start, len;

          cregex->patterns[i].regexbuf.not_eol = 0;
          if (0 <= (start = re_search (&cregex->patterns[i].regexbuf, beg,
                                       end - beg, 0,
                                       end - beg, &cregex->patterns[i].regs)))
            {
              len = cregex->patterns[i].regs.end[0] - start;
              if (exact)
                {
                  *match_size = len;
                  return start;
                }
              if (cregex->match_lines)
                {
                  if (len == end - beg) /* implies start == 0 */
                    goto success;
                }
              else if (cregex->match_words)
                {
                  /* If -w, check if the match aligns with word boundaries.
                     We do this iteratively because:
                     (a) the line may contain more than one occurence of the
                         pattern, and
                     (b) Several alternatives in the pattern might be valid at
                         a given point, and we may need to consider a shorter
                         one to find a word boundary.  */
                  while (start >= 0)
                    {
                      if ((start == 0 || !IS_WORD_CONSTITUENT ((unsigned char) beg[start - 1]))
                          && (start + len == end - beg
                              || !IS_WORD_CONSTITUENT ((unsigned char) beg[start + len])))
                        goto success;
                      if (len > 0)
                        {
                          /* Try a shorter length anchored at the same place. */
                          --len;
                          cregex->patterns[i].regexbuf.not_eol = 1;
                          len = re_match (&cregex->patterns[i].regexbuf, beg,
                                          start + len, start,
                                          &cregex->patterns[i].regs);
                        }
                      if (len <= 0)
                        {
                          /* Try looking further on. */
                          if (start == end - beg)
                            break;
                          ++start;
                          cregex->patterns[i].regexbuf.not_eol = 0;
                          start = re_search (&cregex->patterns[i].regexbuf, beg,
                                             end - beg,
                                             start, end - beg - start,
                                             &cregex->patterns[i].regs);
                          len = cregex->patterns[i].regs.end[0] - start;
                        }
                    }
                }
              else
                goto success;
            }
        }

      if (end < buflim)
        end++;
    }
  return (size_t) -1;

 success:
  *match_size = end - beg;
  return beg - buf;
}

static void
EGfree (void *compiled_pattern)
{
  struct compiled_regex *cregex = (struct compiled_regex *) compiled_pattern;

  free (cregex->patterns);
  free (cregex);
}

/* POSIX Basic Regular Expressions */
matcher_t matcher_grep =
  {
    Gcompile,
    EGexecute,
    EGfree
  };

/* POSIX Extended Regular Expressions */
matcher_t matcher_egrep =
  {
    Ecompile,
    EGexecute,
    EGfree
  };

/* AWK Regular Expressions */
matcher_t matcher_awk =
  {
    AWKcompile,
    EGexecute,
    EGfree
  };
