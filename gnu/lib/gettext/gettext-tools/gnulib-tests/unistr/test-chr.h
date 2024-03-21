/* Test of uN_chr() functions.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

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

/* Written by Eric Blake and Bruno Haible <bruno@clisp.org>, 2010.  */

int
main (void)
{
  size_t size = 0x100000;
  size_t length;
  UNIT *input;
  uint32_t *input32 = (uint32_t *) malloc (size * sizeof (uint32_t));
  ASSERT (input32);

  input32[0] = 'a';
  input32[1] = 'b';
  u32_set (input32 + 2, 'c', 1024);
  for (size_t i = 1026; i < size - 2; i += 63)
    {
      size_t last = i + 63 < size - 2 ? i + 63 : size - 2;
      ucs4_t uc = 'd' | (i - 1026);
      if (uc >= 0xd800 && uc <= 0xdfff)
        uc |= 0x100000;
      u32_set (input32 + i, uc, last - i);
    }

  input32[size - 2] = 'e';
  input32[size - 1] = 'a';

  input = U32_TO_U (input32, size, NULL, &length);
  ASSERT (input);

  /* Basic behavior tests.  */
  ASSERT (U_CHR (input, length, 'a') == input);

  ASSERT (U_CHR (input, 0, 'a') == NULL);
  {
    void *page_boundary = zerosize_ptr ();
    if (page_boundary != NULL)
      ASSERT (U_CHR (page_boundary, 0, 'a') == NULL);
  }

  ASSERT (U_CHR (input, length, 'b') == input + 1);
  ASSERT (U_CHR (input, length, 'c') == input + 2);
  ASSERT (U_CHR (input, length, 'd') == input + 1026);

  {
    UNIT *exp = input + 1026;
    UNIT *prev = input + 1;
    for (size_t i = 1026; i < size - 2; i += 63)
      {
        UNIT c[6];
        size_t n;
        ucs4_t uc = 'd' | (i - 1026);
        if (uc >= 0xd800 && uc <= 0xdfff)
          uc |= 0x100000;
        n = U_UCTOMB (c, uc, 6);
        ASSERT (exp < input + length - 1);
        ASSERT (U_CHR (prev, (length - 1) - (prev - input), uc) == exp);
        ASSERT (memcmp (exp, c, n * sizeof (UNIT)) == 0);
        prev = exp;
        exp += n * 63;
      }
  }

  ASSERT (U_CHR (input + 1, length - 1, 'a') == input + length - 1);
  ASSERT (U_CHR (input + 1, length - 1, 'e') == input + length - 2);

  ASSERT (U_CHR (input, length, 'f') == NULL);
  ASSERT (U_CHR (input, length, '\0') == NULL);

  /* Check that a very long haystack is handled quickly if the byte is
     found near the beginning.  */
  {
    size_t repeat = 10000;
    for (; repeat > 0; repeat--)
      {
        ASSERT (U_CHR (input, length, 'c') == input + 2);
      }
  }

  /* Alignment tests.  */
  {
    int i, j;
    for (i = 0; i < 32; i++)
      {
        for (j = 0; j < 128; j++)
          input[i + j] = j;
        for (j = 0; j < 128; j++)
          {
            ASSERT (U_CHR (input + i, 128, j) == input + i + j);
          }
      }
  }

  /* Check that uN_chr() does not read past the first occurrence of the
     byte being searched.  */
  {
    UNIT *page_boundary = zerosize_ptr ();
    size_t n;

    if (page_boundary != NULL)
      {
        for (n = 1; n <= 500 / sizeof (UNIT); n++)
          {
            UNIT *mem = page_boundary - n;
            U_SET (mem, 'X', n);
            ASSERT (U_CHR (mem, n, 'U') == NULL);

            {
              size_t i;

              for (i = 0; i < n; i++)
                {
                  mem[i] = 'U';
                  ASSERT (U_CHR (mem, 4000, 'U') == mem + i);
                  mem[i] = 'X';
                }
            }
          }
      }
  }

  free (input);
  if (sizeof (UNIT) != sizeof (uint32_t))
    free (input32);

  return 0;
}
