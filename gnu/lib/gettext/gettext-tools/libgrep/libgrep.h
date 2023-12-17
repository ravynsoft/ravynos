/* Search for patterns in strings or files.
   Copyright (C) 2005 Free Software Foundation, Inc.

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

#ifndef _LIBGREP_H
#define _LIBGREP_H

#include <stdbool.h>
#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif


/* A pattern matcher.  */
typedef struct {

  /* Compile a pattern and return the compiled pattern.  */
  void * (*compile) (const char *pattern, size_t pattern_size,
                     bool match_icase, bool match_words, bool match_lines,
                     char eolbyte);

  /* Execute a search.  */
  size_t (*execute) (const void *compiled_pattern,
                     const char *buf, size_t buf_size,
                     size_t *match_size, bool exact);

  /* Free a compiled pattern.  */
  void (*free) (void *compiled_pattern);

} matcher_t;

/* The built-in pattern matchers.  */
extern matcher_t matcher_grep;   /* POSIX Basic Regular Expressions */
extern matcher_t matcher_egrep;  /* POSIX Extended Regular Expressions */
extern matcher_t matcher_fgrep;  /* Fixed String search */
extern matcher_t matcher_awk;    /* AWK Regular Expressions */


#ifdef __cplusplus
}
#endif


#endif /* _LIBGREP_H */
