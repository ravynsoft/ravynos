/* Test of string descriptors.
   Copyright (C) 2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2023.  */

#include <config.h>

#include "string-desc.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "macros.h"

int
main (int argc, char *argv[])
{
  ASSERT (argc > 1);
  int fd3 = open (argv[1], O_RDWR | O_TRUNC | O_CREAT, 0600);
  ASSERT (fd3 >= 0);

  string_desc_t s0 = string_desc_new_empty ();
  string_desc_t s1 = string_desc_from_c ("Hello world!");
  string_desc_t s2 = string_desc_new_addr (21, "The\0quick\0brown\0\0fox");

  /* Test string_desc_length.  */
  ASSERT (string_desc_length (s0) == 0);
  ASSERT (string_desc_length (s1) == 12);
  ASSERT (string_desc_length (s2) == 21);

  /* Test string_desc_char_at.  */
  ASSERT (string_desc_char_at (s1, 0) == 'H');
  ASSERT (string_desc_char_at (s1, 11) == '!');
  ASSERT (string_desc_char_at (s2, 0) == 'T');
  ASSERT (string_desc_char_at (s2, 1) == 'h');
  ASSERT (string_desc_char_at (s2, 2) == 'e');
  ASSERT (string_desc_char_at (s2, 3) == '\0');
  ASSERT (string_desc_char_at (s2, 4) == 'q');
  ASSERT (string_desc_char_at (s2, 15) == '\0');
  ASSERT (string_desc_char_at (s2, 16) == '\0');

  /* Test string_desc_data.  */
  (void) string_desc_data (s0);
  ASSERT (memcmp (string_desc_data (s1), "Hello world!", 12) == 0);
  ASSERT (memcmp (string_desc_data (s2), "The\0quick\0brown\0\0fox", 21) == 0);

  /* Test string_desc_is_empty.  */
  ASSERT (string_desc_is_empty (s0));
  ASSERT (!string_desc_is_empty (s1));
  ASSERT (!string_desc_is_empty (s2));

  /* Test string_desc_startswith.  */
  ASSERT (string_desc_startswith (s1, s0));
  ASSERT (!string_desc_startswith (s0, s1));
  ASSERT (!string_desc_startswith (s1, s2));
  ASSERT (!string_desc_startswith (s2, s1));
  ASSERT (string_desc_startswith (s2, string_desc_from_c ("The")));
  ASSERT (string_desc_startswith (s2, string_desc_new_addr (9, "The\0quick")));
  ASSERT (!string_desc_startswith (s2, string_desc_new_addr (9, "The\0quirk")));

  /* Test string_desc_endswith.  */
  ASSERT (string_desc_endswith (s1, s0));
  ASSERT (!string_desc_endswith (s0, s1));
  ASSERT (!string_desc_endswith (s1, s2));
  ASSERT (!string_desc_endswith (s2, s1));
  ASSERT (!string_desc_endswith (s2, string_desc_from_c ("fox")));
  ASSERT (string_desc_endswith (s2, string_desc_new_addr (4, "fox")));
  ASSERT (string_desc_endswith (s2, string_desc_new_addr (6, "\0\0fox")));
  ASSERT (!string_desc_endswith (s2, string_desc_new_addr (5, "\0\0ox")));

  /* Test string_desc_cmp.  */
  ASSERT (string_desc_cmp (s0, s0) == 0);
  ASSERT (string_desc_cmp (s0, s1) < 0);
  ASSERT (string_desc_cmp (s0, s2) < 0);
  ASSERT (string_desc_cmp (s1, s0) > 0);
  ASSERT (string_desc_cmp (s1, s1) == 0);
  ASSERT (string_desc_cmp (s1, s2) < 0);
  ASSERT (string_desc_cmp (s2, s0) > 0);
  ASSERT (string_desc_cmp (s2, s1) > 0);
  ASSERT (string_desc_cmp (s2, s2) == 0);

  /* Test string_desc_index.  */
  ASSERT (string_desc_index (s0, 'o') == -1);
  ASSERT (string_desc_index (s2, 'o') == 12);

  /* Test string_desc_last_index.  */
  ASSERT (string_desc_last_index (s0, 'o') == -1);
  ASSERT (string_desc_last_index (s2, 'o') == 18);

  /* Test string_desc_contains.  */
  ASSERT (string_desc_contains (s0, string_desc_from_c ("ll")) == -1);
  ASSERT (string_desc_contains (s1, string_desc_from_c ("ll")) == 2);
  ASSERT (string_desc_contains (s1, string_desc_new_addr (1, "")) == -1);
  ASSERT (string_desc_contains (s2, string_desc_new_addr (1, "")) == 3);
  ASSERT (string_desc_contains (s1, string_desc_new_addr (2, "\0")) == -1);
  ASSERT (string_desc_contains (s2, string_desc_new_addr (2, "\0")) == 15);

  /* Test string_desc_substring.  */
  ASSERT (string_desc_cmp (string_desc_substring (s1, 2, 5),
                           string_desc_from_c ("llo")) == 0);

  /* Test string_desc_write.  */
  ASSERT (string_desc_write (fd3, s0) == 0);
  ASSERT (string_desc_write (fd3, s1) == 0);
  ASSERT (string_desc_write (fd3, s2) == 0);

  /* Test string_desc_fwrite.  */
  ASSERT (string_desc_fwrite (stdout, s0) == 0);
  ASSERT (string_desc_fwrite (stdout, s1) == 0);
  ASSERT (string_desc_fwrite (stdout, s2) == 0);

  /* Test string_desc_new, string_desc_set_char_at, string_desc_fill.  */
  string_desc_t s4;
  ASSERT (string_desc_new (&s4, 5) == 0);
  string_desc_set_char_at (s4, 0, 'H');
  string_desc_set_char_at (s4, 4, 'o');
  string_desc_set_char_at (s4, 1, 'e');
  string_desc_fill (s4, 2, 4, 'l');
  ASSERT (string_desc_length (s4) == 5);
  ASSERT (string_desc_startswith (s1, s4));

  /* Test string_desc_new_filled, string_desc_set_char_at.  */
  string_desc_t s5;
  ASSERT (string_desc_new_filled (&s5, 5, 'l') == 0);
  string_desc_set_char_at (s5, 0, 'H');
  string_desc_set_char_at (s5, 4, 'o');
  string_desc_set_char_at (s5, 1, 'e');
  ASSERT (string_desc_length (s5) == 5);
  ASSERT (string_desc_startswith (s1, s5));

  /* Test string_desc_equals.  */
  ASSERT (!string_desc_equals (s1, s5));
  ASSERT (string_desc_equals (s4, s5));

  /* Test string_desc_copy, string_desc_free.  */
  {
    string_desc_t s6;
    ASSERT (string_desc_copy (&s6, s0) == 0);
    ASSERT (string_desc_is_empty (s6));
    string_desc_free (s6);
  }
  {
    string_desc_t s6;
    ASSERT (string_desc_copy (&s6, s2) == 0);
    ASSERT (string_desc_equals (s6, s2));
    string_desc_free (s6);
  }

  /* Test string_desc_overwrite.  */
  {
    string_desc_t s7;
    ASSERT (string_desc_copy (&s7, s2) == 0);
    string_desc_overwrite (s7, 4, s1);
    ASSERT (string_desc_equals (s7, string_desc_new_addr (21, "The\0Hello world!\0fox")));
  }

  /* Test string_desc_concat.  */
  {
    string_desc_t s8;
    ASSERT (string_desc_concat (&s8, 3, string_desc_new_addr (10, "The\0quick"),
                                        string_desc_new_addr (7, "brown\0"),
                                        string_desc_new_addr (4, "fox"),
                                        string_desc_new_addr (7, "unused")) == 0);
    ASSERT (string_desc_equals (s8, s2));
    string_desc_free (s8);
  }

  /* Test string_desc_c.  */
  {
    char *ptr = string_desc_c (s2);
    ASSERT (ptr != NULL);
    ASSERT (memcmp (ptr, "The\0quick\0brown\0\0fox\0", 22) == 0);
    free (ptr);
  }

  close (fd3);

  return 0;
}
