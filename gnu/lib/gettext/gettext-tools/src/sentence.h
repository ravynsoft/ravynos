/* Sentence handling.
   Copyright (C) 2015 Free Software Foundation, Inc.
   Written by Daiki Ueno <ueno@gnu.org>, 2015.

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

#ifndef _SENTENCE_H
#define _SENTENCE_H

#include "unitypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The minimal number of white spaces which should follow after the
   end of sentence.  */
extern DLL_VARIABLE int sentence_end_required_spaces;

/* Locate the position of a sentence end marker (a period, a question
   mark, etc) in a null-terminated string STR.  If there is no
   sentence end marker found in STR, return a pointer to the null byte
   at the end of STR.  ENDING_CHARP is a return location of the end
   marker character.  */
extern const char *sentence_end (const char *string, ucs4_t *ending_charp);

#ifdef __cplusplus
}
#endif

#endif  /* _SENTENCE_H */
