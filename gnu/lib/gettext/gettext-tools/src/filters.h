/* Recoding functions.
   Copyright (C) 2006, 2014 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

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

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Convert a string INPUT of INPUT_LEN bytes containing Serbian input
   to Latin script (not Latin language :-)), converting Cyrillic letters to
   Latin letters.
   Store the freshly allocated result in *OUTPUT_P and its length (in bytes)
   in *OUTPUT_LEN_P.
   Input and output are in UTF-8 encoding.  */
extern void serbian_to_latin (const char *input, size_t input_len,
                              char **output_p, size_t *output_len_p);

/* Convert a string INPUT of INPUT_LEN bytes, converting ASCII
   quotations to Unicode quotations.
   Store the freshly allocated result in *OUTPUT_P and its length (in bytes)
   in *OUTPUT_LEN_P.
   Input and output are in UTF-8 encoding.  */
extern void ascii_quote_to_unicode (const char *input, size_t input_len,
                                    char **output_p, size_t *output_len_p);

/* Convert a string INPUT of INPUT_LEN bytes, converting ASCII
   quotations to Unicode quotations, adding bold escape sequence.
   Store the freshly allocated result in *OUTPUT_P and its length (in bytes)
   in *OUTPUT_LEN_P.
   Input and output are in UTF-8 encoding.  */
extern void ascii_quote_to_unicode_bold (const char *input, size_t input_len,
                                         char **output_p, size_t *output_len_p);

#ifdef __cplusplus
}
#endif

