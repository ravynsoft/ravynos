/* Test of uN_cmp() functions.
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

/* Written by Simon Josefsson and Bruno Haible <bruno@clisp.org>, 2010.  */

static void
test_cmp (void)
{
  /* Test equal / not equal distinction.  */
  void *page_boundary1 = zerosize_ptr ();
  void *page_boundary2 = zerosize_ptr ();
  if (page_boundary1 && page_boundary2)
    ASSERT (U_CMP (page_boundary1, page_boundary2, 0) == 0);
  {
    static const UNIT input1[] = { 'f', 'o', 'o', 0 };
    static const UNIT input2[] = { 'f', 'o', 'o', 'b', 'a', 'r', 0 };
    ASSERT (U_CMP (input1, input2, 2) == 0);
    ASSERT (U_CMP (input1, input2, 3) == 0);
    ASSERT (U_CMP (input1, input2, 4) != 0);
  }
  {
    static const UNIT input1[] = { 'f', 'o', 'o', 0 };
    static const UNIT input2[] = { 'b', 'a', 'r', 0 };
    ASSERT (U_CMP (input1, input2, 1) != 0);
    ASSERT (U_CMP (input1, input2, 3) != 0);
  }

  /* Test less / equal / greater distinction.  */
  {
    static const UNIT input1[] = { 'f', 'o', 'o', 0 };
    static const UNIT input2[] = { 'm', 'o', 'o', 0 };
    ASSERT (U_CMP (input1, input2, 4) < 0);
    ASSERT (U_CMP (input2, input1, 4) > 0);
  }
  {
    static const UNIT input1[] = { 'o', 'o', 'm', 'p', 'h', 0 };
    static const UNIT input2[] = { 'o', 'o', 'p', 's', 0 };
    ASSERT (U_CMP (input1, input2, 3) < 0);
    ASSERT (U_CMP (input2, input1, 3) > 0);
  }
  {
    static const UNIT input1[] = { 'f', 'o', 'o', 0 };
    static const UNIT input2[] = { 'f', 'o', 'o', 'b', 'a', 'r', 0 };
    ASSERT (U_CMP (input1, input2, 4) < 0);
    ASSERT (U_CMP (input2, input1, 4) > 0);
  }

  /* Some old versions of memcmp were not 8-bit clean.  */
  {
    static const UNIT input1[] = { 0x40 };
    static const UNIT input2[] = { 0xC2 };
    ASSERT (U_CMP (input1, input2, 1) < 0);
    ASSERT (U_CMP (input2, input1, 1) > 0);
  }
  {
    static const UNIT input1[] = { 0xC2 };
    static const UNIT input2[] = { 0xC3 };
    ASSERT (U_CMP (input1, input2, 1) < 0);
    ASSERT (U_CMP (input2, input1, 1) > 0);
  }

  /* The Next x86 OpenStep bug shows up only when comparing 16 bytes
     or more and with at least one buffer not starting on a 4-byte boundary.
     William Lewis provided this test program.   */
  {
    UNIT foo[21];
    UNIT bar[21];
    int i;
    for (i = 0; i < 4; i++)
      {
        UNIT *a = foo + i;
        UNIT *b = bar + i;
        int j;
        for (j = 0; j < 8; j++)
          a[j] = '-';
        a[8] = '0';
        for (j = 9; j < 16; j++)
          a[j] = '1';
        for (j = 0; j < 8; j++)
          b[j] = '-';
        b[8] = '1';
        for (j = 9; j < 16; j++)
          b[j] = '0';
        ASSERT (U_CMP (a, b, 16) < 0);
      }
  }
}
